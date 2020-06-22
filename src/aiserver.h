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

namespace rockchip {
namespace aiserver {

class AIServer {
public:
  AIServer();
  virtual ~AIServer();

private:
  std::unique_ptr<DBusServer> mDbusServer;
};

} // namespace aiserver
} // namespace rockchip

#endif // _RK_MEIDA_SERVER_H_
