/*
 * Copyright 2019 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * author: martin.cheng@rock-chips.com
 *   date: 2020/06/09
 * module: task graph for generic ai scene.
 */

#include "ai_scene_director.h"

#include "shmc/shm_control.h"

// task graph from rockit.
#include "rockit/RTTaskGraph.h"
#include "rockit/RTMediaBuffer.h"
#include "rockit/RTMediaMetaKeys.h"
#include "rockit/RTMediaRockx.h"

// rockx model names
#define ROCKX_MODEL_FACE_DETECT       "rockx_face_detect"
#define ROCKX_MODEL_FACE_LANDMARK     "rockx_face_landmark"
#define ROCKX_MODEL_POSE_BODY         "rockx_pose_body"

// basic task graph
#define ROCKX_SCENE_SINGLE    "/oem/usr/share/aiserver/camera_nv12_rkrga_300_rknn_graph.json"
#define ROCKX_SCENE_COMPLEX   "/oem/usr/share/aiserver/camera_complex.json"

// task graph for subgraph
#define ROCKX_SUBGRAPH_FACE_DECTECT    "/oem/usr/share/aiserver/nv12_rkrga_300_rknn_facedetect.json"
#define ROCKX_SUBGRAPH_POSE_BODY       "/oem/usr/share/aiserver/nv12_rkrga_300_rknn_posebody.json"
#define ROCKX_SUBGRAPH_FACE_LANDMARK   "/oem/usr/share/aiserver/nv12_rkrga_300_rknn_face_landmark.json"

namespace rockchip {
namespace aiserver {

RT_RET nn_data_call_back(RTMediaBuffer *buffer) {
    RTRknnAnalysisResults *nnReply = NULL;
    buffer->getMetaData()->findPointer(ROCKX_OUT_RESULT, reinterpret_cast<RT_PTR *>(&nnReply));
    if (RT_NULL != nnReply) {
        ShmControl::sendNNDataByRndis((void*)nnReply);
    }
    buffer->release();
    return RT_OK;
}

AISceneDirector::AISceneDirector() {
    // shmc cell: shared memory containers for high performance server
    queue_w_.InitForWrite(kShmKey, kQueueBufSize);
    ShmControl::initialize();

    mTaskGraph = nullptr;
}

AISceneDirector::~AISceneDirector() {
    if (nullptr != mTaskGraph) {
        delete mTaskGraph;
        mTaskGraph = nullptr;
    }
}

int32_t AISceneDirector::runNNSingle(const char* uri) {
    mTaskGraph = new RTTaskGraph("ai_app");
    mTaskGraph->autoBuild(ROCKX_SCENE_SINGLE);

    // observer, prepare and start task graph
    mTaskGraph->observeOutputStream("faceDetectOutput", 2, nn_data_call_back);
    mTaskGraph->invoke(GRAPH_CMD_PREPARE, NULL);
    mTaskGraph->invoke(GRAPH_CMD_START, NULL);
    return 0;
}

int32_t AISceneDirector::runNNComplex() {
    mTaskGraph = new RTTaskGraph("ai_app");
    mTaskGraph->autoBuild(ROCKX_SCENE_COMPLEX);

    // prepare and start task graph
    mTaskGraph->invoke(GRAPH_CMD_PREPARE, NULL);
    mTaskGraph->invoke(GRAPH_CMD_START, NULL);
    return 0;
}

int32_t AISceneDirector::ctrlSubGraph(const char* nnName, bool enable) {
    if ((nullptr == mTaskGraph) || (nullptr == nnName)) {
        return -1;
    }

    RT_LOGD("nnName %s, enable: %d", nnName, enable);
    if (!strcmp(ROCKX_MODEL_FACE_DETECT, nnName)) {
        if (mEnabledFaceDetect != enable) {
            ctrlFaceDectect(enable);
            mEnabledFaceDetect = enable;
        }
    } else if (!strcmp(ROCKX_MODEL_FACE_LANDMARK, nnName)) {
        if (mEnabledFaceLandmark != enable) {
            ctrlFaceLandmark(enable);
            mEnabledFaceLandmark = enable;
        }
    } else if (!strcmp(ROCKX_MODEL_POSE_BODY, nnName)) {
        if (mEnabledPoseBody != enable) {
            ctrlFacePoseBody(enable);
            mEnabledPoseBody = enable;
        }
    } else {
        RT_LOGE("unsupport nn data: %s", nnName);
    }

    return 0;
}

int32_t AISceneDirector::ctrlFaceDectect(bool enable) {
    if (enable) {
        mTaskGraph->addSubGraph(ROCKX_SUBGRAPH_FACE_DECTECT);
        mTaskGraph->observeOutputStream("nn:facedetect", 2, nn_data_call_back);
    } else {
        mTaskGraph->removeSubGraph(ROCKX_SUBGRAPH_FACE_DECTECT);
    }
    return 0;
}

int32_t AISceneDirector::ctrlFaceLandmark(bool enable) {
    if (enable) {
        mTaskGraph->addSubGraph(ROCKX_SUBGRAPH_FACE_LANDMARK);
        mTaskGraph->observeOutputStream("nn:faceLandmark", 4, nn_data_call_back);
    } else {
        mTaskGraph->removeSubGraph(ROCKX_SUBGRAPH_FACE_LANDMARK);
    }
    return 0;
}

int32_t AISceneDirector::ctrlFacePoseBody(bool enable) {
    if (enable) {
        mTaskGraph->addSubGraph(ROCKX_SUBGRAPH_POSE_BODY);
        mTaskGraph->observeOutputStream("nn:posebody", 3, nn_data_call_back);
    } else {
        mTaskGraph->removeSubGraph(ROCKX_SUBGRAPH_POSE_BODY);
    }
    return 0;
}

int32_t AISceneDirector::interrupt() {
    if (nullptr != mTaskGraph) {
        mTaskGraph->waitUntilDone();
    }
    return 0;
}

int32_t AISceneDirector::waitUntilDone() {
    if (nullptr != mTaskGraph) {
        mTaskGraph->waitUntilDone();
    }
    return 0;
}

void RTAIGraphListener::CtrlSubGraph(const char* nnName, int32_t enable) {
    if (nullptr != mDirector) {
        mDirector->ctrlSubGraph(nnName, enable);
    }
}

} // namespace aiserver
} // namespace rockchip
