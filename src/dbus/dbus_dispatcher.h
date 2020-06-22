// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_DISPATCHER_H_
#define _RK_DBUS_DISPATCHER_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dbus_media_control_adaptor.h"
#include <dbus-c++/dbus.h>

namespace rockchip {
namespace aiserver {

#define MEDIA_CONTROL_BUS_NAME "rockchip.aiserver.control"
#define MEDIA_CONTROL_ENCODER_PATH "/rockchip/aiserver/control/encoder"
#define MEDIA_CONTROL_CAMERA_PATH "/rockchip/aiserver/control/camera"
#define MEDIA_CONTROL_AUDIO_PATH "/rockchip/aiserver/control/audio"

// define by package dbserver
#define DBSERVE_BUS_NAME "rockchip.dbserver"
#define DBSERVE_PATH "/"
#define DBSERVE_MEDIA_INTERFACE DBSERVE_BUS_NAME ".media"
#define DBSERVE_DBCHANGE_INTERFACE DBSERVE_MEDIA_INTERFACE
#define DBSERVE_EVENT_INTERFACE DBSERVE_BUS_NAME ".event"
#define DBSERVE_DBEVENT_CHANGE_INTERFACE DBSERVE_EVENT_INTERFACE

// define by package storage_manager
#define STORAGE_MANAGER_BUS_NAME "rockchip.StorageManager"
#define STORAGE_MANAGER_PATH "/"
#define STORAGE_MANAGER_MEDIA_INTERFACE STORAGE_MANAGER_BUS_NAME ".file"
#define STORAGE_MANAGER_DBCHANGE_INTERFACE STORAGE_MANAGER_MEDIA_INTERFACE

class DbusDispatcher {
public:
  DbusDispatcher() = delete;
  DbusDispatcher(int timeout_ms);
  virtual ~DbusDispatcher() {}
  friend class DBusDbServer;

private:
  static DBus::BusDispatcher dispatcher_;
  int timeout_ms_;
};
} // namespace aiserver
} // namespace rockchip

#endif // _RK_DBUS_INTERFACE_H_
