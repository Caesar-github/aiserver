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
  INT32 eptz_npu_width;              // AI����ʹ��ͼ��Ŀ�
  INT32 eptz_npu_height;             // AI����ʹ��ͼ��ĸ�
  INT32 eptz_src_width;              //����ͷԴ���ݵĿ�
  INT32 eptz_src_height;             //����ͷԴ���ݵĸ�
  INT32 eptz_dst_width;              //����ͷ������ʾ�ֱ��ʵĿ�
  INT32 eptz_dst_height;             //����ͷ������ʾ�ֱ��ʵĸ�
  INT32 eptz_threshold_x;            //��������X������
  INT32 eptz_threshold_y;            //��������Y������
  INT32 eptz_iterate_x;              //��������X�����ٶ�
  INT32 eptz_iterate_y;              //��������Y�����ٶ�
  float eptz_clip_ratio;             //��������ZOOM/PANЧ���ֱ��ʱ�������
  float eptz_facedetect_score_shold; // AI������������ֵ
  INT32 eptz_fast_move_frame_judge;  //�����ƶ�������ֵ
  INT32 eptz_zoom_speed;             //��������ZOOM/PANЧ��ת���ٶ�
} EptzInitInfo;

EPTZ_RET eptzConfigInit(EptzInitInfo *_eptz_info);
EPTZ_RET calculateClipRect(EptzAiData *eptz_ai_data, INT32 *output_result);
EPTZ_RET calculateClipRect(EptzAiData *eptz_ai_data, INT32 *output_result,
                           bool back_to_origin, int delay_time);

#endif // EPTZ_ALGORITHM_H_
