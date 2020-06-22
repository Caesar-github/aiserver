// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <nn_data.pb.h>
#include <sys/prctl.h>
#include <sys/time.h>

#ifdef USE_RKMEDIA
#include "buffer.h"
#include "flow_common.h"
#include "link_config.h"
#endif

#include "shm_control.h"
#include "rknn_user.h"
#include "logger/log.h"
#include "rockit/RTMediaRockx.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "shm_control"

#ifdef ENABLE_SHM_SERVER
shmc::ShmQueue<shmc::SVIPC> queue_w_;
#endif

namespace ShmControl {
int setNNGeneralInfos(NNData *nnData, void *bufptr, int size) {
    if (!bufptr || size <= 0)
      return -1;
    
    auto nn_result = (RknnResult *)(bufptr);
    int type = nn_result[0].type;
    int npu_w = 0, npu_h = 0;
    npu_w = nn_result[0].img_w;
    npu_h = nn_result[0].img_h;
    if (npu_w <= 0 || npu_h <= 0) {
        npu_w = 300;
        npu_h = 300;
    }
    nnData->set_nn_width(npu_w);
    nnData->set_nn_height(npu_h);
    nnData->set_model_type(type);
    return 0;
}

void pushFaceDetectInfo(NNData *nnData, void *bufptr, int size) {
    if (!bufptr || !nnData)
      return;
    auto nn_result_face_detect = (RknnResult *)(bufptr);
    FaceDetect *facedetect;
    for (int i = 0; i < size; i++) {
        auto face_detect_item = nn_result_face_detect[i].face_info.object;
        float score = face_detect_item.score;
        if (score < 0.5f) {
            printf("face score %f\n", score);
            continue;
        }
        int x1 = face_detect_item.box.left;
        int y1 = face_detect_item.box.top;
        int x2 = face_detect_item.box.right;
        int y2 = face_detect_item.box.bottom;
        facedetect = nnData->add_face_detect();
        facedetect->set_left(x1);
        facedetect->set_top(y1);
        facedetect->set_right(x2);
        facedetect->set_bottom(y2);
        facedetect->set_score(score);
        printf("AIServer: FaceInfo Rect[%04d,%04d,%04d,%04d] score=%f\n", x1, y1, x2, y2, score);
    }
}

void pushPoseBodyInfo(NNData *nnData, void *bufptr, int size) {
    if (!bufptr || !nnData)
        return;
    auto nn_result_pose_body = (RknnResult *)(bufptr);
    LandMark *landmark;
    for (uint32_t i = 0; i < size; i++) {
        auto keyPointsItem = nn_result_pose_body[i].body_info.object;
        landmark = nnData->add_landmark();
        for (int j = 0; j < keyPointsItem.count; j++) {
            Points *mPoint = landmark->add_points();
            mPoint->set_x(keyPointsItem.points[j].x);
            mPoint->set_y(keyPointsItem.points[j].y);
            LOG_INFO("  %s [%d, %d] %f\n", ROCKX_POSE_BODY_KEYPOINTS_NAME[j],
                     keyPointsItem.points[j].x, keyPointsItem.points[j].y,
                     keyPointsItem.score[j]);
        }
    }
}

void pushLandMarkInfo(NNData *nnData, void *bufptr, int size) {
    if (!bufptr || !nnData)
      return;
    auto nn_result_land_mark = (RknnResult *)(bufptr);
    nnData->set_model_type(2);
    LandMark *landmark;
    for (uint32_t i = 0; i < size; i++) {
        landmark = nnData->add_landmark();
        auto face_landmark_item = nn_result_land_mark[i].landmark_info.object;
        for (int j = 0; j < face_landmark_item.landmarks_count; j++) {
            Points *mPoint = landmark->add_points();
            mPoint->set_x(face_landmark_item.landmarks[j].x);
            mPoint->set_y(face_landmark_item.landmarks[j].y);
        }
    }
}

void pushFingerDetectInfo(NNData *nnData, void *bufptr, int size) {
    if (!bufptr)
        return;
}

void PushUserHandler(void *handler, int type, void *buffer, int size) {
    if (!buffer)
        return;
    auto shnnData          = (ShmNNData*)(buffer);
    const char *model_name = shnnData->nn_model_name;
    int  dataSize          = shnnData->size;
    bool hasNNInfoUpdate   = true;
    NNData nnData;
    if (!model_name || !(shnnData->size))
        return;

    if (strstr(model_name, "rockx_face_detect")) {
        pushFaceDetectInfo(&nnData,   shnnData->rknn_result, shnnData->size);
    } else if (strstr(model_name, "rockx_pose_body")) {
        pushPoseBodyInfo(&nnData,     shnnData->rknn_result, shnnData->size);
    } else if (strstr(model_name, "rockx_face_landmark")) {
        pushLandMarkInfo(&nnData,     shnnData->rknn_result, shnnData->size);
    } else if (strstr(model_name, "rockx_pose_finger")) {
        pushFingerDetectInfo(&nnData, shnnData->rknn_result, shnnData->size);
    } else {
        hasNNInfoUpdate = false;
    }

    if (hasNNInfoUpdate == true) {
        std::string sendbuf;
        int res = setNNGeneralInfos(&nnData, shnnData->rknn_result, shnnData->size);
        if (res >= 0) {
            nnData.set_model_name(model_name);
            nnData.SerializeToString(&sendbuf);
            nnData.ParseFromString(sendbuf);
            // printf("nndata model_name=%s\n", nnData.model_name().c_str());
            queue_w_.Push(sendbuf);
        }
    }
}

void sendNNDataToRndis(void* nnReply) {
    RTRknnAnalysisResults *nnResult = (RTRknnAnalysisResults *)nnReply;
    int        nnCount    = nnResult->counter;

    ShmNNData shmData     = {0};
    shmData.timestamp     = 0;
    shmData.size          = nnCount;
    RknnResult nnData[nnCount];

    switch ((RknnResultType)nnResult->results[0].type) {
      case NNRESULT_TYPE_FACE:
        shmData.nn_model_name = "rockx_face_detect";
        break;
      case NNRESULT_TYPE_BODY:
        shmData.nn_model_name = "rockx_pose_body";
        break;
      case NNRESULT_TYPE_LANDMARK:
        shmData.nn_model_name = "rockx_face_landmark";
        break;
      case NNRESULT_TYPE_FINGER:
        shmData.nn_model_name = "rockx_pose_finger";
        break;
      default:
        break;
    }

    for(int idx = 0; idx < nnCount; idx++) {
        nnData[idx].img_w   = 300;
        nnData[idx].img_h   = 300;
        nnData[idx].timeval = 5*1000;
        nnData[idx].status  = SUCCESS;
        nnData[idx].type    = (RknnResultType)nnResult->results[idx].type;

        switch (nnData[idx].type) {
          case NNRESULT_TYPE_FACE: {
            rockx_object_t *nnface = &(nnResult->results[idx].face_info.object);
            nnData[idx].face_info.object.box.left   = nnface->box.left;
            nnData[idx].face_info.object.box.top    = nnface->box.top;
            nnData[idx].face_info.object.box.right  = nnface->box.right;
            nnData[idx].face_info.object.box.bottom = nnface->box.bottom;
            nnData[idx].face_info.object.score = nnface->score;
            }break;
          case NNRESULT_TYPE_BODY:
            memcpy(&(nnResult->results[idx].body_info.object), \
                      &(nnData[idx].body_info.object), sizeof(rockx_keypoints_t));
            break;
          case NNRESULT_TYPE_LANDMARK:
            memcpy(&(nnResult->results[idx].landmark_info.object), \
                      &(nnData[idx].landmark_info.object), sizeof(rockx_face_landmark_t));
            break;
          case NNRESULT_TYPE_FINGER:
            memcpy(&(nnResult->results[idx].finger_info.object), \
                      &(nnData[idx].finger_info.object), sizeof(rockx_keypoints_t));
            break;
          default:
            nnData[idx].status  = FAILURE;
            break;
        }
    }
    if (nnCount > 0) {
        shmData.rknn_result = nnData;
        PushUserHandler(nullptr, 0, (void*)(&shmData), nnCount);
    }
}

void initialize() {
#ifdef ENABLE_SHM_SERVER
    shmc::SetLogHandler(shmc::kDebug, [](shmc::LogLevel lv, const char *s) {
        printf("[%d] %s\n", lv, s);
    });
    queue_w_.InitForWrite(kShmKey, kQueueBufSize);
#endif
}

} // namespace ShmControl
