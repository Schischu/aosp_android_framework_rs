/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include "rsCpuIntrinsicIntraPred.h"

void RsdCpuScriptIntrinsicIntraPred::setGlobalObj(uint32_t slot,
                                                  ObjectBase *data) {
    Allocation *alloc = static_cast<Allocation *>(data);
    if (slot == 0) mIntraParam = (INTRA_PARAM *)alloc->mHal.state.userProvidedPtr;
}

void RsdCpuScriptIntrinsicIntraPred::setGlobalVar(uint32_t slot,
                                                  const void *data,
                                                  size_t dataLength) {
    mBlockCnt = ((int32_t *)data)[0];
}

void RsdCpuScriptIntrinsicIntraPred::kernel(const RsForEachStubParamStruct *p,
                                            uint32_t xstart, uint32_t xend,
                                            uint32_t instep, uint32_t outstep) {
    RsdCpuScriptIntrinsicIntraPred *cp = (RsdCpuScriptIntrinsicIntraPred *)p->usr;

    int i = 0, plane = 0;
    int bsize = 0, index = 0;
    INTRA_PARAM *param;
    for (index = 0; index < cp->mBlockCnt; index++) {
        for (plane = 0; plane < MAX_MB_PLANE; plane++) {
            param = cp->mIntraParam + index * MAX_MB_PLANE + plane;
            bsize = param->bsize;
            const TX_SIZE tx_size = plane ? get_uv_tx_size_rs(param->tx_size, param->sb_type) :
                                    (TX_SIZE)param->tx_size;
            const BLOCK_SIZE plane_bsize = get_plane_block_size_rs((BLOCK_SIZE)bsize, param);
            const int num_4x4_w = num_4x4_blocks_wide_lookup[plane_bsize];
            const int num_4x4_h = num_4x4_blocks_high_lookup[plane_bsize];
            const int step = 1 << (tx_size << 1);
            if (param->mb_to_right_edge < 0 || param->mb_to_bottom_edge < 0) {
                int r, c;
                int max_blocks_wide = num_4x4_w;
                int max_blocks_high = num_4x4_h;

                if (param->mb_to_right_edge < 0) {
                    max_blocks_wide += (param->mb_to_right_edge >> (5 + param->subsampling_x));
                }
                if (param->mb_to_bottom_edge < 0) {
                    max_blocks_high += (param->mb_to_bottom_edge >> (5 + param->subsampling_y));
                }
                i = 0;
                for (r = 0; r < num_4x4_h; r += (1 << tx_size)) {
                    for (c = 0; c < num_4x4_w; c += (1 << tx_size)) {
                        if (r < max_blocks_high && c < max_blocks_wide) {
                            decode_block_intra_recon_rs(plane, i, plane_bsize, tx_size, param);
                        }
                        i += step;
                    }
                }
            } else {
                for (i = 0; i < num_4x4_w * num_4x4_h; i += step) {
                    decode_block_intra_recon_rs(plane, i, plane_bsize, tx_size, param);
                }
            }
        }
    }
}

RsdCpuScriptIntrinsicIntraPred::RsdCpuScriptIntrinsicIntraPred(RsdCpuReferenceImpl *ctx,
                                                               const Script *s, const Element *e)
            : RsdCpuScriptIntrinsic(ctx, s, e, RS_SCRIPT_INTRINSIC_ID_INTRA_PRED) {
    mRootPtr = &kernel;
}

RsdCpuScriptIntrinsicIntraPred::~RsdCpuScriptIntrinsicIntraPred() {
}

void RsdCpuScriptIntrinsicIntraPred::populateScript(Script *s) {
    s->mHal.info.exportedVariableCount = 3;
}

void RsdCpuScriptIntrinsicIntraPred::invokeFreeChildren() {
}


RsdCpuScriptImpl * rsdIntrinsic_IntraPred(RsdCpuReferenceImpl *ctx,
                                          const Script *s, const Element *e) {
    return new RsdCpuScriptIntrinsicIntraPred(ctx, s, e);
}
