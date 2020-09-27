// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ai_task_factory.h"

#ifdef HAVE_STASTERIA
#include "st_task_handler.h"
#elif HAVE_ROCKX
#include "rockx_task_handler.h"
#endif

namespace rockchip {
namespace aiserver {

AITaskHandler* AITaskFactory::createHandler() {
    AITaskHandler *handler = nullptr;
#ifdef HAVE_STASTERIA
    handler = new STTaskHandler();
#elif HAVE_ROCKX
    handler = new RockxTaskHandler();
#endif

    return handler;
}

} // namespace aiserver
} // namespace rockchip
