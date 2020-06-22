/*
 * Copyright 2020 Rockchip Electronics Co. LTD
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
 * author: modified by <martin.cheng@rock-chips.com>
 *   date: 2020-05-23
 *  title: ai server with task graph of rockit
 */

#include <getopt.h>
#include <memory>

#include "aiserver.h"
#include "logger/log.h"
#include "shmc/shm_control.h"

// #include "rockit/RTExecutor.h"
// #include "rockit/RTTaskNodeOptions.h"
#include "rockit/RTMediaBuffer.h"
#include "rockit/RTMediaMetaKeys.h"
#include "rockit/RTMediaRockx.h"
#include "nn_vision_rockx.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "aiserver.cpp"

#define AI_GRAPH_JSON_PREFIX "/data/"
#define AI_GRAPH_JSON "aiserver.json"

#define USE_ROCKIT 1

#define AI_APP_GRAPH_CONFIG_FILE    "/oem/usr/share/aiserver/camera.json"
#define FACEDETECT_CONFIG_FILE      "/oem/usr/share/aiserver/nv12_rkrga_300_rknn_facedetect.json"
#define POSEBODY_CONFIG_FILE        "/oem/usr/share/aiserver/nv12_rkrga_300_rknn_posebody.json"
#define FACE_LANDMARK_CONFIG_FILE   "/oem/usr/share/aiserver/nv12_rkrga_300_rknn_face_landmark.json"

typedef struct _AIServerCtx {
    int mFlagMinilog;
    int mFlagMinilogBacktrace;
    int mFlagMinilogLevel;
    int mFlagEncoderDebug;
    bool mQuit;
    bool mNeedDbus;
    bool mNeedDbserver;
    bool mSessionBus;
    std::string mJsonUri;
} AIServerCtx;

static AIServerCtx _ai_server_ctx;

namespace rockchip {
namespace aiserver {

#define ROCKX_MODEL_FACE_DETECT       "rockx_face_detect"
#define ROCKX_MODEL_FACE_LANDMARK     "rockx_face_landmark"
#define ROCKX_MODEL_POSE_BODY         "rockx_pose_body"

RT_RET nn_data_call_back(RTMediaBuffer *buffer) {
    RTRknnAnalysisResults *nnReply = NULL;
    buffer->getMetaData()->findPointer(ROCKX_OUT_RESULT, reinterpret_cast<RT_PTR *>(&nnReply));
    if (RT_NULL != nnReply) {
        ShmControl::sendNNDataByRndis((void*)nnReply);
    }
    buffer->release();
    return RT_OK;
}

RTAICameraGraph::RTAICameraGraph() {
    mGraph = new RTTaskGraph("ai camera");
    mGraph->autoBuild(AI_APP_GRAPH_CONFIG_FILE);
}

RTAICameraGraph::~RTAICameraGraph() {
    rt_safe_delete(mGraph);
}

void RTAICameraGraph::start() {
    mGraph->invoke(GRAPH_CMD_START, NULL);
}

void RTAICameraGraph::prepare() {
    mGraph->invoke(GRAPH_CMD_PREPARE, NULL);
}

void RTAICameraGraph::waitUntilDone() {
    mGraph->waitUntilDone();
}

void RTAICameraGraph::enableFaceDetect(int32_t enable) {
    if (enable) {
        mGraph->addSubGraph(FACEDETECT_CONFIG_FILE);
        mGraph->observeOutputStream("facedetectOutput", 2, nn_data_call_back);
    } else {
        mGraph->removeSubGraph(FACEDETECT_CONFIG_FILE);
    }
}

void RTAICameraGraph::enableFaceLandmark(int32_t enable) {
    if (enable) {
        mGraph->addSubGraph(FACE_LANDMARK_CONFIG_FILE);
        mGraph->observeOutputStream("faceLandmarkOutput", 4, nn_data_call_back);
    } else {
        mGraph->removeSubGraph(FACE_LANDMARK_CONFIG_FILE);
    }
}

void RTAICameraGraph::enablePoseBody(int32_t enable) {
    if (enable) {
        mGraph->addSubGraph(POSEBODY_CONFIG_FILE);
        mGraph->observeOutputStream("posebodyOutput", 3, nn_data_call_back);
    } else {
        mGraph->removeSubGraph(POSEBODY_CONFIG_FILE);
    }
}

void RTAIGraphListener::CtrlSubGraph(const char* nnName, int32_t enable) {
    RT_LOGD("nnName %s, enable: %d", nnName, enable);
    if (!strcmp(ROCKX_MODEL_FACE_DETECT, nnName)) {
        if (mFaceDetectEnabled != enable) {
            mGraph->enableFaceDetect(enable);
            mFaceDetectEnabled = enable;
        }
    } else if (!strcmp(ROCKX_MODEL_FACE_LANDMARK, nnName)) {
        if (mFaceLandmarkEnabled != enable) {
            mGraph->enableFaceLandmark(enable);
            mFaceLandmarkEnabled = enable;
        }
    } else if (!strcmp(ROCKX_MODEL_POSE_BODY, nnName)) {
        if (mPoseBodyEnabled != enable) {
            mGraph->enablePoseBody(enable);
            mPoseBodyEnabled = enable;
        }
    } else {
        RT_LOGE("unsupport nn data: %s", nnName);
    }
}

AIServer::AIServer() {
    mGraph = new RTAICameraGraph();
    mGraphListener = new RTAIGraphListener(mGraph);

    RT_LOGD("AIServer: ctx.mNeedDbus     = %d", _ai_server_ctx.mNeedDbus);
    RT_LOGD("AIServer: ctx.mNeedDbserver = %d", _ai_server_ctx.mNeedDbserver);

    if (_ai_server_ctx.mNeedDbus) {
        mDbusServer.reset(new DBusServer(_ai_server_ctx.mSessionBus, _ai_server_ctx.mNeedDbserver));
        assert(mDbusServer);
        mDbusServer->RegisterMediaControl(mGraphListener);
        mDbusServer->start();
    }

    // push nndata to uvc TV app by shm protocal.
    ShmControl::initialize();

    // aiApp->observeOutputStream("faceDetectOutput", 2, nn_data_call_back);
    
    mGraph->prepare();
    mGraph->start();
}

AIServer::~AIServer() {
    if (_ai_server_ctx.mNeedDbus) {
        if (mDbusServer != nullptr)
        mDbusServer->stop();
    }
    rt_safe_delete(mGraphListener);
    rt_safe_delete(mGraph);
}

void AIServer::waitUntilDone() {
    assert(mGraph);
    mGraph->waitUntilDone();
}

} // namespace aiserver
} // namespace rockchip

static void sigterm_handler(int sig) {
    fprintf(stderr, "signal %d\n", sig);
    _ai_server_ctx.mQuit = true;
}

int main(int argc, char *argv[]) {
    // parse_args(argc, argv);

    // install signal handlers.
#if USE_ROCKIT
    // signal(SIGQUIT, sigterm_handler);
    // signal(SIGINT,  sigterm_handler);
    // signal(SIGTERM, sigterm_handler);
    // signal(SIGXCPU, sigterm_handler);
    // signal(SIGPIPE, SIG_IGN);

    _ai_server_ctx.mQuit         = false;
    _ai_server_ctx.mNeedDbus     = true;
    _ai_server_ctx.mNeedDbserver = false;
    _ai_server_ctx.mSessionBus   = false;
#endif

    // __minilog_log_init(argv[0], NULL, false, _ai_server_ctx.mFlagMinilogBacktrace, argv[0], "1.0.0");
    std::unique_ptr<rockchip::aiserver::AIServer> aiserver =
                std::unique_ptr<rockchip::aiserver::AIServer>(new rockchip::aiserver::AIServer());

    aiserver->waitUntilDone();
#if 0
    while (!_ai_server_ctx.mQuit) {
       std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
#endif
    aiserver.reset();

    return 0;
}

