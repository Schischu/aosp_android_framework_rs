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

#ifndef RSD_CPU_SCRIPT_INTRINSIC_INTRA_PRED_H
#define RSD_CPU_SCRIPT_INTRINSIC_INTRA_PRED_H

#include "rsCpuIntrinsic.h"
#include "rsCpuIntrinsicInlines.h"
#include "intra/vp9_intra.h"
#include "intra/vp9_blockd.h"
#define MAX_TILE 4

using namespace android;
using namespace android::renderscript;

namespace android {
namespace renderscript {

class RsdCpuScriptIntrinsicIntraPred: public RsdCpuScriptIntrinsic {
public:
    virtual void populateScript(Script *);
    virtual void invokeFreeChildren();

    virtual void setGlobalObj(uint32_t slot, ObjectBase *data);
    virtual void setGlobalVar(uint32_t slot, const void *data, size_t dataLength);
    virtual ~RsdCpuScriptIntrinsicIntraPred();
    RsdCpuScriptIntrinsicIntraPred(RsdCpuReferenceImpl *ctx, const Script *s, const Element *e);

protected:
    INTRA_PARAM *mIntraParam;
    int mBlockCnt;
    static void kernel(const RsForEachStubParamStruct *p,
                       uint32_t xstart, uint32_t xend,
                       uint32_t instep, uint32_t outstep);
};

}
}
#endif
