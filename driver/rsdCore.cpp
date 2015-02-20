/*
 * Copyright (C) 2011-2012 The Android Open Source Project
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

#include "../cpu_ref/rsd_cpu.h"

#include "rsdCore.h"
#include "rsdAllocation.h"
#include "rsdBcc.h"
#include "rsdElement.h"
#include "rsdType.h"
#ifndef RS_COMPATIBILITY_LIB
    #include "rsdGL.h"
    #include "rsdPath.h"
    #include "rsdProgramStore.h"
    #include "rsdProgramRaster.h"
    #include "rsdProgramVertex.h"
    #include "rsdProgramFragment.h"
    #include "rsdMesh.h"
    #include "rsdFrameBuffer.h"
#else
    #include <dlfcn.h>
#endif
#include "rsdSampler.h"
#include "rsdScriptGroup.h"

#include <malloc.h>
#include "rsContext.h"

#include <sys/types.h>
#include <sys/resource.h>
#include <sched.h>
#include <sys/syscall.h>
#include <string.h>

using namespace android;
using namespace android::renderscript;

static void Shutdown(Context *rsc);
static void SetPriority(const Context *rsc, int32_t priority);

#ifndef RS_COMPATIBILITY_LIB
    #define NATIVE_FUNC(a) a
#else
    #define NATIVE_FUNC(a) nullptr
#endif

static RsdHalFunctions FunctionTable = {
    NATIVE_FUNC(rsdGLInit),
    NATIVE_FUNC(rsdGLShutdown),
    NATIVE_FUNC(rsdGLSetSurface),
    NATIVE_FUNC(rsdGLSwap),

    Shutdown,
    nullptr,
    SetPriority,
    rsdAllocRuntimeMem,
    rsdFreeRuntimeMem,
    {
        rsdScriptInit,
        rsdInitIntrinsic,
        rsdScriptInvokeFunction,
        rsdScriptInvokeRoot,
        rsdScriptInvokeForEach,
        rsdScriptInvokeInit,
        rsdScriptInvokeFreeChildren,
        rsdScriptSetGlobalVar,
        rsdScriptGetGlobalVar,
        rsdScriptSetGlobalVarWithElemDims,
        rsdScriptSetGlobalBind,
        rsdScriptSetGlobalObj,
        rsdScriptDestroy,
        rsdScriptInvokeForEachMulti,
        rsdScriptUpdateCachedObject
    },

    {
        rsdAllocationInit,
        rsdAllocationAdapterInit,
        rsdAllocationDestroy,
        rsdAllocationGrallocBits,
        rsdAllocationResize,
        rsdAllocationSyncAll,
        rsdAllocationMarkDirty,
        NATIVE_FUNC(rsdAllocationSetSurface),
        NATIVE_FUNC(rsdAllocationIoSend),
        NATIVE_FUNC(rsdAllocationIoReceive),
        rsdAllocationData1D,
        rsdAllocationData2D,
        rsdAllocationData3D,
        rsdAllocationRead1D,
        rsdAllocationRead2D,
        rsdAllocationRead3D,
        rsdAllocationLock1D,
        rsdAllocationUnlock1D,
        rsdAllocationData1D_alloc,
        rsdAllocationData2D_alloc,
        rsdAllocationData3D_alloc,
        rsdAllocationElementData1D,
        rsdAllocationElementData2D,
        rsdAllocationElementRead1D,
        rsdAllocationElementRead2D,
        rsdAllocationGenerateMipmaps,
        rsdAllocationUpdateCachedObject,
        rsdAllocationAdapterOffset
    },


    {
        NATIVE_FUNC(rsdProgramStoreInit),
        NATIVE_FUNC(rsdProgramStoreSetActive),
        NATIVE_FUNC(rsdProgramStoreDestroy)
    },

    {
        NATIVE_FUNC(rsdProgramRasterInit),
        NATIVE_FUNC(rsdProgramRasterSetActive),
        NATIVE_FUNC(rsdProgramRasterDestroy)
    },

    {
        NATIVE_FUNC(rsdProgramVertexInit),
        NATIVE_FUNC(rsdProgramVertexSetActive),
        NATIVE_FUNC(rsdProgramVertexDestroy)
    },

    {
        NATIVE_FUNC(rsdProgramFragmentInit),
        NATIVE_FUNC(rsdProgramFragmentSetActive),
        NATIVE_FUNC(rsdProgramFragmentDestroy)
    },

    {
        NATIVE_FUNC(rsdMeshInit),
        NATIVE_FUNC(rsdMeshDraw),
        NATIVE_FUNC(rsdMeshDestroy)
    },

    {
        NATIVE_FUNC(rsdPathInitStatic),
        NATIVE_FUNC(rsdPathInitDynamic),
        NATIVE_FUNC(rsdPathDraw),
        NATIVE_FUNC(rsdPathDestroy)
    },

    {
        rsdSamplerInit,
        rsdSamplerDestroy,
        rsdSamplerUpdateCachedObject
    },

    {
        NATIVE_FUNC(rsdFrameBufferInit),
        NATIVE_FUNC(rsdFrameBufferSetActive),
        NATIVE_FUNC(rsdFrameBufferDestroy)
    },

    {
        rsdScriptGroupInit,
        rsdScriptGroupSetInput,
        rsdScriptGroupSetOutput,
        rsdScriptGroupExecute,
        rsdScriptGroupDestroy,
        nullptr
    },

    {
        rsdTypeInit,
        rsdTypeDestroy,
        rsdTypeUpdateCachedObject
    },

    {
        rsdElementInit,
        rsdElementDestroy,
        rsdElementUpdateCachedObject
    },

    nullptr // finish
};

extern const RsdCpuReference::CpuSymbol * rsdLookupRuntimeStub(Context * pContext, char const* name);

static RsdCpuReference::CpuScript * LookupScript(Context *, const Script *s) {
    return (RsdCpuReference::CpuScript *)s->mHal.drv;
}

#ifdef RS_COMPATIBILITY_LIB
typedef void (*sAllocationDestroyFnPtr) (const Context *rsc, Allocation *alloc);
typedef void (*sAllocationIoSendFnPtr) (const Context *rsc, Allocation *alloc);
typedef void (*sAllocationSetSurfaceFnPtr) (const Context *rsc, Allocation *alloc, ANativeWindow *nw);
static sAllocationDestroyFnPtr sAllocationDestroy;
static sAllocationIoSendFnPtr sAllocationIoSend;
static sAllocationSetSurfaceFnPtr sAllocationSetSurface;

static bool loadIOSuppLibSyms() {
    void* handleIO = nullptr;
    handleIO = dlopen("libRSSupportIO.so", RTLD_LAZY | RTLD_LOCAL);
    if (handleIO == nullptr) {
        ALOGE("Couldn't load libRSSupportIO.so");
        return false;
    }
    sAllocationDestroy = (sAllocationDestroyFnPtr)dlsym(handleIO, "rscAllocationDestroy");
    if (sAllocationDestroy==nullptr) {
        ALOGE("Failed to initialize sAllocationDestroy");
        return false;
    }
    sAllocationIoSend = (sAllocationIoSendFnPtr)dlsym(handleIO, "rscAllocationIoSend");
    if (sAllocationIoSend==nullptr) {
        ALOGE("Failed to initialize sAllocationIoSend");
        return false;
    }
    sAllocationSetSurface = (sAllocationSetSurfaceFnPtr)dlsym(handleIO, "rscAllocationSetSurface");
    if (sAllocationSetSurface==nullptr) {
        ALOGE("Failed to initialize sAllocationIoSend");
        return false;
    }
    return true;
}
#endif

extern "C" bool rsdHalInit(RsContext c, uint32_t version_major,
                           uint32_t version_minor) {
    Context *rsc = (Context*) c;
#ifdef RS_COMPATIBILITY_LIB
    if (loadIOSuppLibSyms()) {
        FunctionTable.allocation.destroy = sAllocationDestroy;
        FunctionTable.allocation.ioSend = sAllocationIoSend;
        FunctionTable.allocation.setSurface = sAllocationSetSurface;
    }
#endif
    rsc->mHal.funcs = FunctionTable;
    RsdHal *dc = (RsdHal *)calloc(1, sizeof(RsdHal));
    if (!dc) {
        ALOGE("Calloc for driver hal failed.");
        return false;
    }
    rsc->mHal.drv = dc;

    dc->mCpuRef = RsdCpuReference::create(rsc, version_major, version_minor,
                                          &rsdLookupRuntimeStub, &LookupScript);
    if (!dc->mCpuRef) {
        ALOGE("RsdCpuReference::create for driver hal failed.");
        rsc->mHal.drv = nullptr;
        free(dc);
        return false;
    }

#ifndef RS_COMPATIBILITY_LIB
    // Set a callback for compiler setup here.
    if (false) {
        dc->mCpuRef->setSetupCompilerCallback(nullptr);
    }
#endif

    return true;
}


void SetPriority(const Context *rsc, int32_t priority) {
    RsdHal *dc = (RsdHal *)rsc->mHal.drv;

    dc->mCpuRef->setPriority(priority);

#ifndef RS_COMPATIBILITY_LIB
    if (dc->mHasGraphics) {
        rsdGLSetPriority(rsc, priority);
    }
#endif
}

void Shutdown(Context *rsc) {
    RsdHal *dc = (RsdHal *)rsc->mHal.drv;
    delete dc->mCpuRef;
    free(dc);
    rsc->mHal.drv = nullptr;
}

void* rsdAllocRuntimeMem(size_t size, uint32_t flags) {
    void* buffer = calloc(size, sizeof(char));
    return buffer;
}

void rsdFreeRuntimeMem(void* ptr) {
    free(ptr);
}
