// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_GRAPH_CONTROL_H_
#define _RK_DBUS_GRAPH_CONTROL_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <memory>

#include "dbus_dispatcher.h"

namespace rockchip {
namespace aiserver {

class RTGraphListener {
 public:
    virtual ~RTGraphListener() {}
    virtual void CtrlSubGraph(const char* nnName, int32_t enable) = 0;
};

class DBusGraphControl : public control::graph_adaptor,
                         public DBus::IntrospectableAdaptor,
                         public DBus::ObjectAdaptor {
 public:
    DBusGraphControl() = delete;
    DBusGraphControl(DBus::Connection &connection, RTGraphListener* listener);
    virtual ~DBusGraphControl();

 public:
    int32_t SetGraphStatus(const std::string &nnName, const int32_t &enabled);
    int32_t SetRockxStatus(const std::string &nnName);

private:
    RTGraphListener* mGraphListener;
};

} // namespace aiserver
} // namespace rockchip

#endif // _RK_DBUS_GRAPH_CONTROL_H_