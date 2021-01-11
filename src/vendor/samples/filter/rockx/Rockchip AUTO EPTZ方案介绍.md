



# Rockchip AUTO EPTZ方案介绍

文件标识：RK-XX-XX-0000

发布版本：V1.0.0

日期：2021-01-11

文件密级：□绝密   □秘密   □内部资料   ■公开

---

**免责声明**

本文档按“现状”提供，瑞芯微电子股份有限公司（“本公司”，下同）不对本文档的任何陈述、信息和内容的准确性、可靠性、完整性、适销性、特定目的性和非侵权性提供任何明示或暗示的声明或保证。本文档仅作为使用指导的参考。

由于产品版本升级或其他原因，本文档将可能在未经任何通知的情况下，不定期进行更新或修改。

**商标声明**

“Rockchip”、“瑞芯微”、“瑞芯”均为本公司的注册商标，归本公司所有。

本文档可能提及的其他所有注册商标或商标，由其各自拥有者所有。

**版权所有** **© 2021 **瑞芯微电子股份有限公司**

超越合理使用范畴，非经本公司书面许可，任何单位和个人不得擅自摘抄、复制本文档内容的部分或全部，并不得以任何形式传播。

瑞芯微电子股份有限公司

Rockchip Electronics Co., Ltd.

地址：     福建省福州市铜盘路软件园A区18号

网址：     [www.rock-chips.com](http://www.rock-chips.com)

客户服务电话： +86-4007-700-590

客户服务传真： +86-591-83951833

客户服务邮箱： [fae@rock-chips.com](mailto:fae@rock-chips.com)

---

**读者对象**

本文档主要适用于以下工程师：

- 技术支持工程师
- 软件开发工程师

**修订记录**

| **日期**   | **版本** | **作者** | **修改说明** |
| ---------- | -------- | -------- | ------------ |
| 2021/01/11 | 1.0.0    | 林其浩   | 初始版本     |
|            |          |          |              |
|            |          |          |              |



**目 录**

[TOC]



## 概述

Rockchip Linux平台支持EPTZ电子云台功能，指通过软件手段，结合智能识别技术实现预览界面的“数字平移- 倾斜- 缩放/变焦”功能。配合RK ROCKX人脸检测算法，快速实现预览画面人物聚焦功能，可应用于视屏会议等多种场景。

### EPTZ功能验证

------

RV1126/RV1109使用EPTZ功能，需将dts中的otp节点使能，evb默认配置中已将其使能。

```shell
&otp {
status = "okay";
};
```

在RV1126/RV1109中，提供三种方案进行AUTO EPTZ功能验证及使用。

* 环境变量：在启动脚本（例如：RkLunch.sh）中添加环境变量export ENABLE_EPTZ=1，默认开启EPTZ功能，
  在所有预览条件下都将启用人脸跟随效果。

* XU控制：通过UVC扩展协议，参考5.1中描述进行实现。当uvc_app接收到XU的CMD_SET_EPTZ(0x0a)指令
  时，将根据指令中所带的int参数1或0，进行EPTZ功能的开关，以确认下次预览时是否开启人脸跟随效果。

* dbus指令：最新版本已支持通过dbus指令通知aiserver进程跨进程动态启动AUTO EPTZ能力：

  ```shell
  #开启命令
  dbus-send --system --print-reply --type=method_call --dest=rockchip.aiserver.control
  /rockchip/aiserver/control/graph rockchip.aiserver.control.graph.EnableEPTZ int32:1
  #关闭命令
  dbus-send --system --print-reply --type=method_call --dest=rockchip.aiserver.control
  /rockchip/aiserver/control/graph rockchip.aiserver.control.graph.EnableEPTZ int32:0
  ```

RV1126/RV1109显示预期效果:

* 单人：在camera可视范围内，尽可能将人脸保持在画面中间。
* 多人：在camera可视范围内，尽可能的显示人多画面，且将其保持在画面中间。

## EPTZ算法集成说明

EPTZ模块支持库为libeptz.so，通过对EptzInitInfo结构体进行配置，实现相应的操作。相关代码位于SDK以下路径：

```
app/aiserver/src/vendor/samples/filter/eptz/
```

具体接口可参考eptz_algorithm.h文件，eptz版本说明详见app\aiserver\src\vendor\samples\filter\eptz\release_note.txt。

## 人脸检测算法集成说明

### ROCKX算法模型替换

------

SDK默认使用rockx_face_detect_v3模型，但rockx同时有提供rockx_face_detect_v2模型，两者数据对比如下：

| 场景                                                        | 识别有效距离 | AI数据帧率 | DDR带宽  | aiserver Uss 内存 |
| ----------------------------------------------------------- | ------------ | ---------- | -------- | ----------------- |
| 1080mjpeg 预览<br />scale1 720 nn<br />rockx_face_detect_v2 | 5米左右      | 10fps      | 3172MB/s | 106740K           |
| 1080mjpeg 预览<br />scale1 720 nn<br />rockx_face_detect_v3 | 2米左右      | 30fps      | 2881MB/s | 105596K           |

EPTZ对AI数据帧率要求不高，因此近距离场景建议使用rockx_face_detect_v3模型，较远距离场景使用rockx_face_detect_v2模型。

SDK默认使用rockx_face_detect_v3，模型替换为rockx_face_detect_v2需执行以下步骤：

* 将external/rockx/目录下的rockx_face_detect_v2.data打包到usr/lib或oem/usr/lib目录下

  ```shell
  diff --git a/sdk/rockx-rv1109-Linux/RockXConfig.cmake b/sdk/rockx-rv1109-Linux/RockXConfig.cmake
  index dd77dc7..151ed97 100644
  --- a/sdk/rockx-rv1109-Linux/RockXConfig.cmake
  +++ b/sdk/rockx-rv1109-Linux/RockXConfig.cmake
  @@ -39,7 +39,7 @@ if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109")
           set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/carplate_recognition.data")
       endif()
       if(${WITH_ROCKX_FACE_DETECTION})
  -        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/face_detection_v3.data")
  +        set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/face_detection_v2.data")
       endif()
       if(${WITH_ROCKX_FACE_RECOGNITION})
           set(ROCKX_DATA_FILES  ${ROCKX_DATA_FILES} "${CMAKE_CURRENT_LIST_DIR}/../rockx-data-rv1109/face_recognition.data")
  ```

* 修改app/aiserver/src/vendor/CMakeLists.txt

  ```shell
  diff --git a/src/vendor/CMakeLists.txt b/src/vendor/CMakeLists.txt
  index d3c8774..ffd177a 100755
  --- a/src/vendor/CMakeLists.txt
  +++ b/src/vendor/CMakeLists.txt
  @@ -25,7 +25,7 @@ if (${ENABLE_SAMPLE_NODE_EPTZ})
       )
   endif()
   
  -option(ENABLE_SAMPLE_NODE_ROCKX  "enable sample node rockx" OFF)
  +option(ENABLE_SAMPLE_NODE_ROCKX  "enable sample node rockx" ON)
  ```

修改后重新编译rockx模块、aiserver模块即可。

后续可以通过修改aicamera.json文件node_4、node_11中的opt_rockx_model，替换算法模型进行效果验证，可选模型包括以下几类，推荐rockx_face_detect_v2和rockx_face_detect_v3模型：

* rockx_head_detect
* rockx_face_detect_v2
* rockx_face_detect_v3
* rockx_face_detect_v3_large

若运行时提示 xxx model data not found, 需将对应模型文件从external/rockx/sdk/rockx-data-rv1109/下拷贝到/usr/lib或oem/usr/lib目录下。

备注：若sensor为2K以上分辨率，建议将RTNodeVFilterEptzDemo.cpp中的eptz_npu_width和eptz_npu_height修改为1280和720，同时将aicamera.json中的node_2节点opt_width、opt_height、opt_vir_width、opt_vir_height修改为1280、720、1280、720，可以提高人脸检测的识别率和准备率。

### 第三方检测算法集成

------

第三方算法集成，可参考external/rockit/doc/《Rockchip_Developer_Guide_Linux_Rockit_CN.pdf》文档进行开发。

具体代码demo可参考以下目录：

```shell
app/aiserver/src/vendor/samples/filter/rockx/
```

结合EPTZ功能使用时，需注意RTNodeVFilterEptzDemo.cpp中传入的AI数据结构要同步修改。