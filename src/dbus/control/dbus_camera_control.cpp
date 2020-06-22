// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_camera_control.h"
#include "flow_export.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_camera_control.cpp"

namespace rockchip {
namespace aiserver {

DBusCameraControl::DBusCameraControl(DBus::Connection &connection)
    : DBus::ObjectAdaptor(connection, MEDIA_CONTROL_CAMERA_PATH) {}

DBusCameraControl::~DBusCameraControl() {}

int32_t DBusCameraControl::SetFrameRate(const int32_t &id,
                                        const int32_t &param) {
  LOG_INFO("DBusCameraControl::SetFrameRate\n");
  camera_control_ = GetCameraControl(id);
  if (camera_control_ == nullptr) {
    LOG_INFO("CameraControl::SetFrameRate id %d is no exist\n", id);
    return -1;
  }
  // TODO
  return 0;
}

int32_t DBusCameraControl::SetResolution(const int32_t &id,
                                         const int32_t &param1,
                                         const int32_t &param2) {
  LOG_INFO("DBusCameraControl::SetResolution\n");
  // TODO
  return 0;
}

int32_t DBusCameraControl::StartCamera(const int32_t &id) {
  LOG_INFO("DBusCameraControl::StartCamera\n");
  // TODO
  return 0;
}

int32_t DBusCameraControl::StopCamera(const int32_t &id) {
  LOG_INFO("DBusCameraControl::StopCamera\n");
  // TODO
  return 0;
}

int32_t DBusCameraControl::TakePicture(const int32_t &id,
                                       const int32_t &count) {
  LOG_INFO("DBusCameraControl::TakePicture\n");
  return TakePhoto(id, count);
}

int32_t DBusCameraControl::TakePicture(const std::string &id_count) {
  LOG_INFO("DBusCameraControl::TakePicture\n");
  int pos = id_count.find("_");
  int id = atoi(id_count.substr(0, pos).c_str());
  int count = atoi(id_count.substr(pos + 1, id_count.size() - 1).c_str());
  LOG_INFO("DBusCameraControl::TakePicture id is %d\n", id);
  LOG_INFO("DBusCameraControl::TakePicture count is %d\n", count);
  return TakePhoto(id, count);
}

} // namespace aiserver
} // namespace rockchip
