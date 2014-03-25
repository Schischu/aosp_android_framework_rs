/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
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
#include "vp9_blockd.h"

int is_inter_block_rs(const int ref_frame) {
  return ref_frame > INTRA_FRAME;
}

TX_SIZE get_uv_tx_size_impl(TX_SIZE y_tx_size, BLOCK_SIZE bsize) {
  if (bsize < BLOCK_8X8) {
    return TX_4X4;
  } else {
    // TODO(dkovalev): Assuming YUV420 (ss_x == 1, ss_y == 1)
    const BLOCK_SIZE plane_bsize = ss_size_lookup[bsize][1][1];
    return MIN(y_tx_size, max_txsize_lookup[plane_bsize]);
  }
}

int b_width_log2(BLOCK_SIZE sb_type) {
  return b_width_log2_lookup[sb_type];
}

TX_TYPE get_tx_type_4x4_rs(PLANE_TYPE plane_type,
                           const INTRA_PARAM *param, int ib) {

  if (plane_type != PLANE_TYPE_Y || param->lossless
          || is_inter_block_rs(param->ref_frame[0]))
    return DCT_DCT;

  return mode2txfm_map[param->sb_type < BLOCK_8X8 ? param->as_mode[ib]
                                                 : param->mode];
}

TX_TYPE get_tx_type_8x8_rs(PLANE_TYPE plane_type,
                           const INTRA_PARAM *param) {
  return plane_type == PLANE_TYPE_Y ? mode2txfm_map[param->mode]
                                    : DCT_DCT;
}

TX_TYPE get_tx_type_16x16_rs(PLANE_TYPE plane_type,
                             const INTRA_PARAM *param) {
  return plane_type == PLANE_TYPE_Y ? mode2txfm_map[param->mode]
                                    : DCT_DCT;
}

TX_SIZE get_uv_tx_size_rs(int tx_size, int sb_type) {
  return get_uv_tx_size_impl((TX_SIZE)tx_size, (BLOCK_SIZE)sb_type);
}

BLOCK_SIZE get_plane_block_size_rs(BLOCK_SIZE bsize,
                                   const INTRA_PARAM *param) {
  BLOCK_SIZE bs = ss_size_lookup[bsize][param->subsampling_x][param->subsampling_y];
  return bs;
}

void extend_for_intra_rs(INTRA_PARAM *param, BLOCK_SIZE plane_bsize,
                         int plane, int aoff, int loff) {
  uint8_t *const buf = param->dst;
  const int stride = param->stride;
  const int x = aoff * 4 - 1;
  const int y = loff * 4 - 1;
  if (param->mb_to_right_edge < 0) {
    const int bw = 4 * num_4x4_blocks_wide_lookup[plane_bsize];
    const int umv_border_start = bw + (param->mb_to_right_edge >>
                                       (3 + param->subsampling_x));

    if (x + bw > umv_border_start)
      memset(&buf[y * stride + umv_border_start],
                 buf[y * stride + umv_border_start - 1], bw);
  }

  if (param->mb_to_bottom_edge < 0) {
    if (param->left_available || x >= 0) {
      const int bh = 4 * num_4x4_blocks_high_lookup[plane_bsize];
      const int umv_border_start =
          bh + (param->mb_to_bottom_edge >> (3 + param->subsampling_y));

      if (y + bh > umv_border_start) {
        const uint8_t c = buf[(umv_border_start - 1) * stride + x];
        uint8_t *d = &buf[umv_border_start * stride + x];
        int i;
        for (i = 0; i < bh; ++i, d += stride)
          *d = c;
      }
    }
  }
}

void txfrm_block_to_raster_xy(BLOCK_SIZE plane_bsize,
                              TX_SIZE tx_size, int block,
                              int *x, int *y) {
  const int bwl = b_width_log2(plane_bsize);
  const int tx_cols_log2 = bwl - tx_size;
  const int tx_cols = 1 << tx_cols_log2;
  const int raster_mb = block >> (tx_size << 1);
  *x = (raster_mb & (tx_cols - 1)) << tx_size;
  *y = (raster_mb >> tx_cols_log2) << tx_size;
}
