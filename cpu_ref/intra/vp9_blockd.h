/*
 *  Copyright (c) 2011 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 *
 * Copyright (C) 2014 The Android Open Source Project
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
 */

#ifndef VP9_COMMON_VP9_BLOCKD_H_
#define VP9_COMMON_VP9_BLOCKD_H_

#include "vp9_rtcd.h"
#include "vp9_enums.h"
#include "vp9_common_data.h"
#include "vp9_common.h"
#include "string.h"
typedef enum {
  PLANE_TYPE_Y  = 0,
  PLANE_TYPE_UV = 1,
  PLANE_TYPES
} PLANE_TYPE;

typedef enum {
  KEY_FRAME = 0,
  INTER_FRAME = 1,
  FRAME_TYPES,
} FRAME_TYPE;

typedef enum {
  DC_PRED,         // Average of above and left pixels
  V_PRED,          // Vertical
  H_PRED,          // Horizontal
  D45_PRED,        // Directional 45  deg = round(arctan(1/1) * 180/pi)
  D135_PRED,       // Directional 135 deg = 180 - 45
  D117_PRED,       // Directional 117 deg = 180 - 63
  D153_PRED,       // Directional 153 deg = 180 - 27
  D207_PRED,       // Directional 207 deg = 180 + 27
  D63_PRED,        // Directional 63  deg = round(arctan(2/1) * 180/pi)
  TM_PRED,         // True-motion
  NEARESTMV,
  NEARMV,
  ZEROMV,
  NEWMV,
  MB_MODE_COUNT
} MB_PREDICTION_MODE;

#define INTRA_MODES (TM_PRED + 1)

typedef struct {
  uint8_t *token_cache;

  int16_t *dqcoeff;
  uint16_t *eobs;
  int plane_type;
  int subsampling_x;
  int subsampling_y;
  uint8_t *dst;
  int stride;

  int mode;
  int uv_mode;
  int ref_frame[2];
  int tx_size;
  unsigned char skip_coeff;
  int sb_type;
  int as_mode[4];


  int up_available;
  int left_available;

  int mb_to_left_edge;
  int mb_to_right_edge;
  int mb_to_top_edge;
  int mb_to_bottom_edge;

  int y_width;
  int y_height;
  int uv_width;
  int uv_height;
  int lossless;
  int bsize;
 }INTRA_PARAM;

typedef enum {
  NONE = -1,
  INTRA_FRAME = 0,
  LAST_FRAME = 1,
  GOLDEN_FRAME = 2,
  ALTREF_FRAME = 3,
  MAX_REF_FRAMES = 4
} MV_REFERENCE_FRAME;

typedef struct {
  MB_PREDICTION_MODE mode, uv_mode;
  MV_REFERENCE_FRAME ref_frame[2];
  TX_SIZE tx_size;
  unsigned char skip_coeff;
  BLOCK_SIZE sb_type;
} MB_MODE_INFO_RS;

int is_inter_block_rs(const int ref_frame);

enum { MAX_MB_PLANE = 3 };

#define BLOCK_OFFSET(x, i) ((x) + (i) * 16)
TX_SIZE get_uv_tx_size_impl(TX_SIZE y_tx_size, BLOCK_SIZE bsize);

int b_width_log2(BLOCK_SIZE sb_type);

extern const TX_TYPE mode2txfm_map[MB_MODE_COUNT];

TX_TYPE get_tx_type_4x4_rs(PLANE_TYPE plane_type,
                           const INTRA_PARAM *param, int ib);

TX_TYPE get_tx_type_8x8_rs(PLANE_TYPE plane_type,
                           const INTRA_PARAM *param);

TX_TYPE get_tx_type_16x16_rs(PLANE_TYPE plane_type,
                             const INTRA_PARAM *param);

void extend_for_intra_rs(INTRA_PARAM *param, BLOCK_SIZE plane_bsize,
                         int plane, int aoff, int loff);

void txfrm_block_to_raster_xy(BLOCK_SIZE plane_bsize,
                              TX_SIZE tx_size, int block,
                              int *x, int *y);
#endif  // VP9_COMMON_VP9_BLOCKD_H_
