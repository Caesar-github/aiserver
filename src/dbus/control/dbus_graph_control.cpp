// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dbus_graph_control.h"

namespace rockchip {
namespace aiserver {

DBusGraphControl::DBusGraphControl(DBus::Connection &connection, RTGraphListener* listener)
                 :DBus::ObjectAdaptor(connection, MEDIA_CONTROL_PATH_GRAPH){
    mGraphListener = listener;
}

DBusGraphControl::~DBusGraphControl() {}

int32_t DBusGraphControl::SetGraphStatus(const std::string &nnName, const int32_t &enabled) {
    printf("Dbus GraphCtol received: %s\n", nnName.c_str());
    if (NULL != mGraphListener) {
        mGraphListener->CtrlSubGraph(nnName.c_str(), enabled);
    }
    return 0;
}

int32_t DBusGraphControl::SetRockxStatus(const std::string &nnName) {
    printf("Dbus GraphCtol received: %s\n", nnName.c_str());
    if (NULL != mGraphListener) {
        mGraphListener->CtrlSubGraph(nnName.c_str(), 1);
    }
    return 0;
}

} // namespace aiserver
} // namespace rockchip
