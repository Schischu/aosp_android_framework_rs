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

#ifndef VP9_BLOCK_INTRA
#define VP9_BLOCK_INTRA
#include "vp9_blockd.h"

extern "C" {
void decode_block_intra_recon_rs(int plane, int block,
                                 BLOCK_SIZE plane_bsize,
                                 TX_SIZE tx_size, INTRA_PARAM *param);
TX_SIZE get_uv_tx_size_rs(int tx_size, int sb_type);
BLOCK_SIZE get_plane_block_size_rs(BLOCK_SIZE bsize,
                                   const INTRA_PARAM *param);
}

#endif
