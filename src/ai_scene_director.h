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

#ifndef _RK_AI_SCENE_DIRECTOR_H_
#define _RK_AI_SCENE_DIRECTOR_H_

#include "dbus_graph_control.h"
#include "rockit/RTTaskGraph.h"

namespace rockchip {
namespace aiserver {

typedef enum _RockxTaskMode {
    ROCKX_TASK_MODE_SINGLE = 0,
    ROCKX_TASK_MODE_COMPLEX,
    ROCKX_TASK_MODE_MAX,
} RockxTaskMode;

/*
 * 1. run task graph for generic ai scene.
 * 2. output NN vision result to SHMC(ipc)
 * 3. provide a minimalist interface to aiserver
 */
class AISceneDirector {
 public:
    AISceneDirector();
    ~AISceneDirector();

 public:
    // control ai scene
    int32_t runNNSingle(const char* nnName);
    int32_t runNNComplex();

 public:
    // control task graph
    int32_t ctrlSubGraph(const char* nnName, bool enable);
    int32_t interrupt();
    int32_t waitUntilDone();

 private:
    int32_t ctrlFaceDectect(bool enable);
    int32_t ctrlFaceLandmark(bool enable);
    int32_t ctrlFacePoseBody(bool enable);

 private:
    RTTaskGraph *mTaskGraph;
    int32_t      mEnabledFaceDetect   = 0;
    int32_t      mEnabledFaceLandmark = 0;
    int32_t      mEnabledPoseBody     = 0;
};

class RTAIGraphListener : public RTGraphListener {
 public:
    RTAIGraphListener(AISceneDirector *director) { mDirector = director; }
    virtual ~RTAIGraphListener() {}
    virtual void CtrlSubGraph(const char* nnName, int32_t enable);
 private:
    AISceneDirector *mDirector;
};

} // namespace aiserver
} // namespace rockchip



#endif // _RK_AI_SCENE_DIRECTOR_H_



