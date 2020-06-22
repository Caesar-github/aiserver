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
#include <logger/log.h>

// headers from this project.
#include "aiserver.h"
#include "ai_scene_director.h"
#include "nn_vision_rockx.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "aiserver.cpp"

#define USE_ROCKIT 1

typedef struct _AIServerCtx {
    int  mFlagMinilog;
    int  mFlagMinilogBacktrace;
    int  mFlagMinilogLevel;
    int  mFlagEncoderDebug;
    int  mTaskMode;
    bool mQuit;
    bool mNeedDbus;
    bool mNeedDbserver;
    bool mSessionBus;
    std::string mJsonUri;
} AIServerCtx;

static AIServerCtx _ai_server_ctx;

namespace rockchip {
namespace aiserver {

AIServer::AIServer() {
    mAIDirector.reset(nullptr);
    mGraphListener.reset(nullptr);
    mDbusServer.reset(nullptr);
}

void AIServer::setupTaskGraph() {
    mAIDirector.reset(new AISceneDirector());
    mGraphListener.reset(new RTAIGraphListener(mAIDirector.get()));

    // RT_LOGD("AIServer: ctx.mNeedDbus     = %d", _ai_server_ctx.mNeedDbus);
    // RT_LOGD("AIServer: ctx.mNeedDbserver = %d", _ai_server_ctx.mNeedDbserver);
    // RT_LOGD("AIServer: ctx.mTaskMode     = %d", _ai_server_ctx.mTaskMode);
    if (_ai_server_ctx.mNeedDbus) {
        mDbusServer.reset(new DBusServer(_ai_server_ctx.mSessionBus, _ai_server_ctx.mNeedDbserver));
        assert(mDbusServer);
        mDbusServer->RegisterMediaControl(mGraphListener.get());
        mDbusServer->start();
    }

    switch (_ai_server_ctx.mTaskMode) {
      case ROCKX_TASK_MODE_SINGLE:
        mAIDirector->runNNSingle("single_model");
        break;
      case ROCKX_TASK_MODE_COMPLEX:
        mAIDirector->runNNComplex();
        break;
      default:
        break;
    }
}

AIServer::~AIServer() {
    if ((_ai_server_ctx.mNeedDbus) && (nullptr != mDbusServer)) {
        mDbusServer->stop();
    }
    mGraphListener.reset();
    mAIDirector.reset();
    mDbusServer.reset();
}

void AIServer::waitUntilDone() {
    mAIDirector->waitUntilDone();
}

} // namespace aiserver
} // namespace rockchip

static void sigterm_handler(int sig) {
    fprintf(stderr, "signal %d\n", sig);
    _ai_server_ctx.mQuit = true;
}

using namespace rockchip::aiserver;

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
    _ai_server_ctx.mTaskMode     = ROCKX_TASK_MODE_SINGLE;
#endif

    // __minilog_log_init(argv[0], NULL, false, _ai_server_ctx.mFlagMinilogBacktrace, argv[0], "1.0.0");
    std::unique_ptr<AIServer> aiserver = std::unique_ptr<AIServer>(new AIServer());
    aiserver->setupTaskGraph();
    aiserver->waitUntilDone();
    aiserver.reset();

    return 0;
}

