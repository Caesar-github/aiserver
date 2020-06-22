// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_MEDIA_SERVER_H_
#define _RK_MEIDA_SERVER_H_

#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <memory>

#include "dbus_server.h"
#include "server.h"
#include "thread.h"
#include "dbus_graph_control.h"
#include "rockit/RTTaskGraph.h"

namespace rockchip {
namespace aiserver {

class RTAICameraGraph {
 public:
    RTAICameraGraph();
    virtual ~RTAICameraGraph();
    void start();
    void prepare();

    void waitUntilDone();

    
    void enableFaceDetect(int32_t enable);
    void enableFaceLandmark(int32_t enable);
    void enablePoseBody(int32_t enable);

 private:
    RTTaskGraph *mGraph;
};

class RTAIGraphListener : public RTGraphListener {
 public:
    RTAIGraphListener(RTAICameraGraph *graph) { mGraph = graph; }
    virtual ~RTAIGraphListener() {}
    virtual void CtrlSubGraph(const char* nnName, int32_t enable);
 private:
    RTAICameraGraph *mGraph;
    int32_t mFaceDetectEnabled = 0;
    int32_t mFaceLandmarkEnabled = 0;
    int32_t mPoseBodyEnabled = 0;
};

class AIServer {
public:
  AIServer();
  virtual ~AIServer();

  void waitUntilDone();
private:
  std::unique_ptr<DBusServer> mDbusServer;
  RTAICameraGraph *mGraph;
  RTGraphListener *mGraphListener;
};

} // namespace aiserver
} // namespace rockchip

#endif // _RK_MEIDA_SERVER_H_
