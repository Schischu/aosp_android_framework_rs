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

#ifndef VP9_COMMON_VP9_COMMON_DATA_H_
#define VP9_COMMON_VP9_COMMON_DATA_H_

#include "vp9_enums.h"

extern const int b_width_log2_lookup[BLOCK_SIZES];
extern const int b_height_log2_lookup[BLOCK_SIZES];
extern const int mi_width_log2_lookup[BLOCK_SIZES];
extern const int mi_height_log2_lookup[BLOCK_SIZES];
extern const int num_8x8_blocks_wide_lookup[BLOCK_SIZES];
extern const int num_8x8_blocks_high_lookup[BLOCK_SIZES];
extern const int num_4x4_blocks_high_lookup[BLOCK_SIZES];
extern const int num_4x4_blocks_wide_lookup[BLOCK_SIZES];
extern const int size_group_lookup[BLOCK_SIZES];
extern const int num_pels_log2_lookup[BLOCK_SIZES];
extern const TX_SIZE max_txsize_lookup[BLOCK_SIZES];
extern const TX_SIZE max_uv_txsize_lookup[BLOCK_SIZES];
extern const TX_SIZE tx_mode_to_biggest_tx_size[TX_MODES];
extern const BLOCK_SIZE ss_size_lookup[BLOCK_SIZES][2][2];

#endif  // VP9_COMMON_VP9_COMMON_DATA_H_
