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

#include "rockit/RTTaskGraph.h"
// #include "rockit/RTExecutor.h"
// #include "rockit/RTTaskNodeOptions.h"
#include "rockit/RTMediaBuffer.h"
#include "rockit/RTMediaMetaKeys.h"
#include "rockit/RTMediaRockx.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "aiserver.cpp"

#define AI_GRAPH_JSON_PREFIX "/data/"
#define AI_GRAPH_JSON "aiserver.json"

#define AI_APP_GRAPH_CONFIG_FILE    "/oem/usr/share/aiserver/camera_nv12_rkrga_300_rknn_graph.json"

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

static void parse_args(int argc, char **argv);

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

AIServer::AIServer() {
    RT_LOGD("AIServer: ctx.mNeedDbus     = %d", _ai_server_ctx.mNeedDbus);
    RT_LOGD("AIServer: ctx.mNeedDbserver = %d", _ai_server_ctx.mNeedDbserver);

    if (_ai_server_ctx.mNeedDbus) {
        mDbusServer.reset(new DBusServer(_ai_server_ctx.mSessionBus, _ai_server_ctx.mNeedDbserver));
        assert(mDbusServer);
        mDbusServer->start();
    }

#if 0
    // install dbus proxy to pipeline
    FlowManagerPtr &flow_manager = FlowManager::GetInstance();
    if (_ai_server_ctx.mNeedDbus) {
        flow_manager->RegisterDBserverProxy(mDbusServer->GetDBserverProxy());
        flow_manager->RegisterDBEventProxy(mDbusServer->GetDBEventProxy());
        flow_manager->RegisterStorageManagerProxy(mDbusServer->GetStorageProxy());
    }
#endif

#if 0
    flow_manager->ConfigParse(_ai_server_ctx.mJsonUri);
    flow_manager->CreatePipes();
#endif

    // push nndata to uvc TV app by shm protocal.
    ShmControl::initialize();

    RTTaskGraph *aiApp = new RTTaskGraph("ai_app");
    aiApp->autoBuild(AI_APP_GRAPH_CONFIG_FILE);
    aiApp->observeOutputStream("faceDetectOutput", 2, nn_data_call_back);
    aiApp->invoke(GRAPH_CMD_PREPARE, NULL);
    aiApp->invoke(GRAPH_CMD_START, NULL);

    aiApp->waitUntilDone();
    rt_safe_delete(aiApp);
}

AIServer::~AIServer() {
    if (_ai_server_ctx.mNeedDbus) {
        if (mDbusServer != nullptr)
        mDbusServer->stop();
    }
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
#if 0
    signal(SIGQUIT, sigterm_handler);
    signal(SIGINT,  sigterm_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGXCPU, sigterm_handler);
    signal(SIGPIPE, SIG_IGN);
#else
    _ai_server_ctx.mQuit         = false;
    _ai_server_ctx.mNeedDbus     = true;
    _ai_server_ctx.mNeedDbserver = true;
    _ai_server_ctx.mSessionBus   = false;
#endif

    // __minilog_log_init(argv[0], NULL, false, _ai_server_ctx.mFlagMinilogBacktrace, argv[0], "1.0.0");
    std::unique_ptr<rockchip::aiserver::AIServer> aiserver =
                std::unique_ptr<rockchip::aiserver::AIServer>(new rockchip::aiserver::AIServer());

#if 0
    while (!_ai_server_ctx.mQuit) {
       std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
#endif
    aiserver.reset();

    return 0;
}

static void usage_tip(FILE *fp, int argc, char **argv) {
    fprintf(fp, "Usage: %s [options]\n"
              "Version %s\n"
              "Options:\n"
              "-c | --config      aiserver confg file \n"
              "-S | --system      Use system bus \n"
              "-s | --session     Use session bus \n"
              "-D | --database    Depend dbserver app \n"
              "-d | --no_database Not depend dbserver app \n"
              "-a | --stand_alone Not depend dbus server \n"
              "-h | --help        For help \n"
              "\n",
            argv[0], "V1.1");
}

static const char short_options[] = "c:SsDdah";
static const struct option long_options[] = {
    {"config",      required_argument, NULL, 'c'},
    {"system",      no_argument,       NULL, 'S'},
    {"session",     no_argument,       NULL, 's'},
    {"database",    no_argument,       NULL, 'D'},
    {"no_database", no_argument,       NULL, 'd'},
    {"stand_alone", no_argument,       NULL, 'a'},
    {"help",        no_argument,       NULL, 'h'},
    {0, 0, 0, 0}
};

static int get_env(const char *name, int *value, int default_value) {
    char *ptr = getenv(name);
    if (NULL == ptr) {
        *value = default_value;
    } else {
        char *endptr;
        int base = (ptr[0] == '0' && ptr[1] == 'x') ? (16) : (10);
        errno = 0;
        *value = strtoul(ptr, &endptr, base);
        if (errno || (ptr == endptr)) {
            errno = 0;
            *value = default_value;
        }
    }
    return 0;
}

static void parse_args(int argc, char **argv) {
    _ai_server_ctx.mFlagMinilog          = 0;
    _ai_server_ctx.mFlagMinilogBacktrace = 0;
    _ai_server_ctx.mFlagMinilogLevel     = 0;
    _ai_server_ctx.mFlagEncoderDebug     = 0;
    _ai_server_ctx.mQuit                 = false;
    _ai_server_ctx.mSessionBus           = false;
    _ai_server_ctx.mNeedDbus             = true;
    _ai_server_ctx.mNeedDbserver         = false;

    get_env("ai_flag_minilog",           &(_ai_server_ctx.mFlagMinilog),          0);
    get_env("ai_flag_minilog_backtrace", &(_ai_server_ctx.mFlagMinilogBacktrace), 0);
    get_env("ai_flag_minilog_level",     &(_ai_server_ctx.mFlagMinilogLevel),     1);
    get_env("ai_flag_encoder_debug",     &(_ai_server_ctx.mFlagEncoderDebug),     0);

    _ai_server_ctx.mJsonUri = "";
#ifdef AI_GRAPH_JSON_PREFIX
    _ai_server_ctx.mJsonUri.append(AI_GRAPH_JSON_PREFIX).append(AI_GRAPH_JSON);
#else
    _ai_server_ctx.mJsonUri.append(AI_GRAPH_JSON);
#endif

    for (;;) {
      int idx;
      int c = getopt_long(argc, argv, short_options, long_options, &idx);
      if (-1 == c) { break; }
    
      switch (c) {
        case 0: /* getopt_long() flag */
          break;
        case 'c':
          _ai_server_ctx.mJsonUri = optarg;
          break;
        case 'S':
          _ai_server_ctx.mSessionBus = false;
          break;
        case 's':
          _ai_server_ctx.mSessionBus = true;
          break;
        case 'D':
          _ai_server_ctx.mNeedDbserver = true;
          break;
        case 'd':
          _ai_server_ctx.mNeedDbserver = false;
          break;
        case 'a':
          _ai_server_ctx.mNeedDbus = false;
          break;
        case 'h':
          usage_tip(stdout, argc, argv);
          exit(EXIT_SUCCESS);
        default:
          usage_tip(stderr, argc, argv);
          exit(EXIT_FAILURE);
      }
    }

    if (_ai_server_ctx.mJsonUri.empty()) {
        usage_tip(stderr, argc, argv);
        exit(EXIT_FAILURE);
    }
}
