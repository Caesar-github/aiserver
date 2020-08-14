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

#include "logger/log.h"

#include "shm_control.h"
#include "nn_vision_rockx.h"

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

    auto nn_result = (RTRknnResult *)(bufptr);
    int type = nn_result->type;
    int npu_w = 0, npu_h = 0;
    npu_w = nn_result->img_w;
    npu_h = nn_result->img_h;
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
    auto nn_result_face_detect = (RTRknnResult *)(bufptr);
    FaceDetect *facedetect;
    for (int i = 0; i < size; i++) {
        auto face_detect_item = nn_result_face_detect->info_face.object;
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
        // printf("AIServer: FaceInfo Rect[%04d,%04d,%04d,%04d] score=%f\n", x1, y1, x2, y2, score);
    }
}

void pushPoseBodyInfo(NNData *nnData, void *bufptr, int size) {
    if (!bufptr || !nnData)
        return;
    auto nn_result_pose_body = (RTRknnResult *)(bufptr);
    LandMark *landmark;
    for (uint32_t i = 0; i < size; i++) {
        auto keyPointsItem = nn_result_pose_body->info_body.object;
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
    auto nn_result_land_mark = (RTRknnResult *)(bufptr);
    LandMark *landmark;
    for (uint32_t i = 0; i < size; i++) {
        landmark = nnData->add_landmark();
        auto face_landmark_item = nn_result_land_mark->info_landmark.object;
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

void sendNNDataByRndis(void* nnDataBuffer) {
    if (!nnDataBuffer)
        return;

    NNData nnData;
    auto   nnResults    = (RTRknnAnalysisResults*)(nnDataBuffer);
    bool   needUpate    = true;
    const char *nnName  = NULL;

    if (!nnResults || !(nnResults->count) || (nnResults->results == NULL))
        return;

    int32_t size = nnResults->count;
    for (int i = 0; i < size; i++) {
        RTRknnResult* result = &nnResults->results[i];
        RTNNDataType nnType = result->type;
        switch (nnType) {
          case RT_NN_TYPE_FACE:
            pushFaceDetectInfo(&nnData, result, 1);
            nnName = ROCKX_MODEL_FACE_DETECT;
            break;
          case RT_NN_TYPE_BODY:
            pushPoseBodyInfo(&nnData, result, 1);
            nnName = ROCKX_MODEL_POSE_BODY;
            break;
          case RT_NN_TYPE_LANDMARK:
            pushLandMarkInfo(&nnData, result, 1);
            nnName = ROCKX_MODEL_FACE_LANDMARK;
            break;
          case RT_NN_TYPE_FINGER:
            pushFingerDetectInfo(&nnData, result, 1);
            nnName = ROCKX_MODEL_POSE_FINGER;
            break;
          default:
            needUpate = false;
            break;
        }
    }

    if (needUpate) {
        std::string sendbuf;
        int res = setNNGeneralInfos(&nnData, &nnResults->results[0], 1);
        if (res >= 0) {
            nnData.set_model_name(nnName);
            nnData.SerializeToString(&sendbuf);
            nnData.ParseFromString(sendbuf);
            // printf("RNDIS send nndata(%s) to RNDIS host.\n", model_name);
            queue_w_.Push(sendbuf);
        }
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
