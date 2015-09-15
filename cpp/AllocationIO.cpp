/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include "RenderScript.h"
#include "rsCppInternal.h"

#if !defined(RS_SERVER) && !defined(RS_COMPATIBILITY_LIB)
#include <gui/Surface.h>
#endif

using namespace android;

#ifndef RS_COMPATIBILITY_LIB
RSC::sp<Surface> RSC::Allocation::getSurface() {
    IGraphicBufferProducer *v = (IGraphicBufferProducer *)RSC::RS::dispatch->AllocationGetSurface(mRS->getContext(),
                                                                                         getID());
    android::sp<IGraphicBufferProducer> bp = v;
    v->decStrong(nullptr);
    RSC::sp<Surface> s = new Surface(bp);
    return s;
}

/*
void Allocation::setSurface() {
    if ((mUsage & RS_ALLOCATION_USAGE_IO_INPUT) == 0) {
        mRS->throwError(RS_ERROR_INVALID_PARAMETER, "Can only send buffer if IO_OUTPUT usage specified.");
        return;
    }
    tryDispatch(mRS, RS::dispatch->AllocationIoReceive(mRS->getContext(), getID()));
}
*/
#endif

