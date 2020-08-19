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

#define HAVE_SIGNAL_PROC 1

typedef struct _AIServerCtx {
    // minilog flags
    int  mFlagMinilog;
    int  mFlagMinilogBacktrace;
    int  mFlagMinilogLevel;
    int  mFlagEncoderDebug;
    // dbus flags
    int  mFlagDBusServer;
    int  mFlagDBusDbServer;
    int  mFlagDBusConn;
    // task graph
    int  mTaskMode;
    // server flags
    bool mQuit;
    std::string mConfigUri;
} AIServerCtx;

static AIServerCtx _ai_server_ctx;
static void        parse_args(int argc, char **argv);

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

    RT_LOGD("AIServer: ctx.mFlagDBusServer   = %d", _ai_server_ctx.mFlagDBusServer);
    RT_LOGD("AIServer: ctx.mFlagDBusDbServer = %d", _ai_server_ctx.mFlagDBusDbServer);
    RT_LOGD("AIServer: ctx.mTaskMode         = %d", _ai_server_ctx.mTaskMode);
    if (_ai_server_ctx.mFlagDBusServer) {
        mDbusServer.reset(new DBusServer(_ai_server_ctx.mFlagDBusConn, _ai_server_ctx.mFlagDBusDbServer));
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
    if ((_ai_server_ctx.mFlagDBusServer) && (nullptr != mDbusServer)) {
        mDbusServer->stop();
    }
    mGraphListener.reset();
    mAIDirector.reset();
    mDbusServer.reset();
}

void AIServer::interrupt() {
    mAIDirector->interrupt();
}

void AIServer::waitUntilDone() {
    mAIDirector->waitUntilDone();
}

} // namespace aiserver
} // namespace rockchip

using namespace rockchip::aiserver;

static AIServer* _ai_server_instance = nullptr;
static void sigterm_handler(int sig) {
    RT_LOGD("quit signal(%d) is caught.", sig);
    _ai_server_ctx.mQuit = true;
    if (nullptr != _ai_server_instance) {
        _ai_server_instance->interrupt();
    }
}

int main(int argc, char *argv[]) {
    _ai_server_ctx.mQuit             = false;
    _ai_server_ctx.mFlagDBusServer   = true;
    _ai_server_ctx.mFlagDBusDbServer = false;
    _ai_server_ctx.mFlagDBusConn     = false;

    parse_args(argc, argv);

    RT_LOGD("parse_args done!");

    // __minilog_log_init(argv[0], NULL, false, _ai_server_ctx.mFlagMinilogBacktrace,
    //                   argv[0], "1.0.0");

    // install signal handlers.
#if HAVE_SIGNAL_PROC
    signal(SIGINT,  sigterm_handler);  // SIGINT  = 2
    signal(SIGQUIT, sigterm_handler);  // SIGQUIT = 3
    signal(SIGTERM, sigterm_handler);  // SIGTERM = 15
    signal(SIGXCPU, sigterm_handler);  // SIGXCPU = 24
    signal(SIGPIPE, SIG_IGN);          // SIGPIPE = 13 is ingnored
#endif

    // __minilog_log_init(argv[0], NULL, false, _ai_server_ctx.mFlagMinilogBacktrace, argv[0], "1.0.0");
    _ai_server_instance = new AIServer();
    RT_LOGD("create aiserver instance done");

    // rt_mem_record_reset();
    _ai_server_instance->setupTaskGraph();
    RT_LOGD("aiserver->setupTaskGraph(); done!");

    _ai_server_instance->waitUntilDone();
    RT_LOGD("aiserver->waitUntilDone(); done!");
    delete _ai_server_instance;
    _ai_server_instance = nullptr;

    rt_mem_record_dump();
    return 0;
}

static void usage_tip(FILE *fp, int argc, char **argv) {
  fprintf(fp, "Usage: %s [options]\n"
              "Version %s\n"
              "Options:\n"
              "-c | --config      AIServer confg file \n"
              "-m | --mode        0:single,  1:complex \n"
              "-o | --dbus_conn   0:system,  1:session \n"
              "-d | --dbus_db     0:disable, 1:enable \n"
              "-s | --dbus_server 0:disable, 1:enable \n"
              "-h | --help        For help \n"
              "\n",
          argv[0], "V1.1");
}

static const char short_options[] = "c:modsh";
static const struct option long_options[] = {
    {"ai_config",   required_argument, NULL, 'c'},
    {"ai_mode",     optional_argument, 0,    'm'},
    {"dbus_conn",   optional_argument, 0,    'o'},
    {"dbus_db",     optional_argument, 0,    'd'},
    {"dbus_server", optional_argument, 0,    's'},
    {"help",        no_argument,       0,    'h'},
    {0, 0, 0, 0}
};

static void parse_args(int argc, char **argv) {
    int opt;
    int idx;
    while ((opt = getopt_long(argc, argv, short_options, long_options, &idx))!= -1) {
      switch (opt) {
        case 0: /* getopt_long() flag */
          break;
        case 'c':
          _ai_server_ctx.mConfigUri = optarg;
          break;
        case 'm':
          _ai_server_ctx.mTaskMode  = atoi(argv[optind]);
          break;
        case 'o':
          _ai_server_ctx.mFlagDBusConn  = atoi(argv[optind]);
          break;
        case 'd':
          _ai_server_ctx.mFlagDBusDbServer = atoi(argv[optind]);
          break;
        case 's':
          _ai_server_ctx.mFlagDBusServer = atoi(argv[optind]);
          break;
        case 'h':
          usage_tip(stdout, argc, argv);
          exit(EXIT_SUCCESS);
        default:
          usage_tip(stderr, argc, argv);
          exit(EXIT_FAILURE);
      }
    }
}

