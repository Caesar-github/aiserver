// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <cstring>

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
    printf("%s: Dbus GraphCtol received: %s\n", __FUNCTION__, nnName.c_str());
    char nnTypeName[64];
    memset(nnTypeName, 0, 64);
    const char *s = nullptr;
    if (!(s = strstr(nnName.c_str(), ":"))) {
        printf("string trans rect format error, string=%s", nnName.c_str());
        return -1;
    }
    strncpy(nnTypeName, nnName.c_str(), s - nnName.c_str());
    int32_t enable = atoi(s + 1);
    if (NULL != mGraphListener) {
        mGraphListener->CtrlSubGraph(nnTypeName, enable);
    }
    return 0;
}

} // namespace aiserver
} // namespace rockchip
