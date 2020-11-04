/*
 * Copyright 2020 Rockchip Electronics Co. LTD
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
 * author: kevin.lin@rock-chips.com
 *   date: 2020-09-18
 * module: eptz
 */

#ifndef EPTZ_ALGORITHM_H_
#define EPTZ_ALGORITHM_H_

#include "eptz_type.h"

typedef struct _EptzInitInfo {
  INT32 eptz_npu_width;              // AI计算使用图像的宽
  INT32 eptz_npu_height;             // AI计算使用图像的高
  INT32 eptz_src_width;              //摄像头源数据的宽
  INT32 eptz_src_height;             //摄像头源数据的高
  INT32 eptz_dst_width;              //摄像头最终显示分辨率的宽
  INT32 eptz_dst_height;             //摄像头最终显示分辨率的高
  INT32 eptz_threshold_x;            //人脸跟踪X灵敏度
  INT32 eptz_threshold_y;            //人脸跟踪Y灵敏度
  INT32 eptz_iterate_x;              //人脸跟踪X方向速度
  INT32 eptz_iterate_y;              //人脸跟踪Y方向速度
  float eptz_clip_ratio;             //人脸跟踪ZOOM/PAN效果分辨率比例设置
  float eptz_facedetect_score_shold; // AI人脸质量分阈值
  INT32 eptz_fast_move_frame_judge;  //人物移动防抖阈值
  INT32 eptz_zoom_speed;             //人脸跟踪ZOOM/PAN效果转换速度
} EptzInitInfo;

EPTZ_RET eptzConfigInit(EptzInitInfo *_eptz_info);
EPTZ_RET calculateClipRect(EptzAiData *eptz_ai_data, INT32 *output_result);
EPTZ_RET calculateClipRect(EptzAiData *eptz_ai_data, INT32 *output_result,
                           bool back_to_origin, int delay_time);

#endif // EPTZ_ALGORITHM_H_
