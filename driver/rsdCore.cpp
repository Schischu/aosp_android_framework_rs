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
    #include "MemChunk.h"
    #include "rsdGL.h"
    #include "rsdPath.h"
    #include "rsdProgramStore.h"
    #include "rsdProgramRaster.h"
    #include "rsdProgramVertex.h"
    #include "rsdProgramFragment.h"
    #include "rsdMesh.h"
    #include "rsdFrameBuffer.h"
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
    #define NATIVE_FUNC(a) NULL
#endif


void setupFunctionTable (RsdHalFunctions* FunctionTable) {
    FunctionTable->initGraphics = NATIVE_FUNC(rsdGLInit);
    FunctionTable->shutdownGraphics = NATIVE_FUNC(rsdGLShutdown);
    FunctionTable->setSurface = NATIVE_FUNC(rsdGLSetSurface);
    FunctionTable->swap = NATIVE_FUNC(rsdGLSwap);
    FunctionTable->shutdownDriver = Shutdown;
    FunctionTable->getVersion = NULL;
    FunctionTable->setPriority = SetPriority;
    FunctionTable->allocRuntimeMem = rsdAllocRuntimeMem;
    FunctionTable->freeRuntimeMem = rsdFreeRuntimeMem;

    FunctionTable->script.init = rsdScriptInit;
    FunctionTable->script.initIntrinsic = rsdInitIntrinsic;
    FunctionTable->script.invokeFunction = rsdScriptInvokeFunction;
    FunctionTable->script.invokeRoot = rsdScriptInvokeRoot;
    FunctionTable->script.invokeForEach = rsdScriptInvokeForEach;
    FunctionTable->script.invokeInit = rsdScriptInvokeInit;
    FunctionTable->script.invokeFreeChildren = rsdScriptInvokeFreeChildren;
    FunctionTable->script.setGlobalVar = rsdScriptSetGlobalVar;
    FunctionTable->script.getGlobalVar = rsdScriptGetGlobalVar;
    FunctionTable->script.setGlobalVarWithElemDims = rsdScriptSetGlobalVarWithElemDims;
    FunctionTable->script.setGlobalBind = rsdScriptSetGlobalBind;
    FunctionTable->script.setGlobalObj = rsdScriptSetGlobalObj;
    FunctionTable->script.destroy = rsdScriptDestroy;
    FunctionTable->script.invokeForEachMulti = rsdScriptInvokeForEachMulti;
    FunctionTable->script.updateCachedObject = rsdScriptUpdateCachedObject;

    FunctionTable->allocation.init = rsdAllocationInit;
    FunctionTable->allocation.destroy = rsdAllocationDestroy;
    FunctionTable->allocation.grallocBits = rsdAllocationGrallocBits;
    FunctionTable->allocation.resize = rsdAllocationResize;
    FunctionTable->allocation.syncAll = rsdAllocationSyncAll;
    FunctionTable->allocation.markDirty = rsdAllocationMarkDirty;
    FunctionTable->allocation.setSurface = NATIVE_FUNC(rsdAllocationSetSurface);
    FunctionTable->allocation.ioSend = NATIVE_FUNC(rsdAllocationIoSend);
    FunctionTable->allocation.ioReceive = NATIVE_FUNC(rsdAllocationIoReceive);
    FunctionTable->allocation.data1D = rsdAllocationData1D;
    FunctionTable->allocation.data2D = rsdAllocationData2D;
    FunctionTable->allocation.data3D = rsdAllocationData3D;
    FunctionTable->allocation.read1D = rsdAllocationRead1D;
    FunctionTable->allocation.read2D = rsdAllocationRead2D;
    FunctionTable->allocation.read3D = rsdAllocationRead3D;
    FunctionTable->allocation.lock1D = rsdAllocationLock1D;
    FunctionTable->allocation.unlock1D = rsdAllocationUnlock1D;
    FunctionTable->allocation.allocData1D = rsdAllocationData1D_alloc;
    FunctionTable->allocation.allocData2D = rsdAllocationData2D_alloc;
    FunctionTable->allocation.allocData3D = rsdAllocationData3D_alloc;
    FunctionTable->allocation.elementData1D = rsdAllocationElementData1D;
    FunctionTable->allocation.elementData2D = rsdAllocationElementData2D;
    FunctionTable->allocation.generateMipmaps = rsdAllocationGenerateMipmaps;
    FunctionTable->allocation.updateCachedObject = rsdAllocationUpdateCachedObject;

    FunctionTable->store.init = NATIVE_FUNC(rsdProgramStoreInit);
    FunctionTable->store.setActive = NATIVE_FUNC(rsdProgramStoreSetActive);
    FunctionTable->store.destroy = NATIVE_FUNC(rsdProgramStoreDestroy);

    FunctionTable->raster.init = NATIVE_FUNC(rsdProgramRasterInit);
    FunctionTable->raster.setActive = NATIVE_FUNC(rsdProgramRasterSetActive);
    FunctionTable->raster.destroy = NATIVE_FUNC(rsdProgramRasterDestroy);

    FunctionTable->vertex.init = NATIVE_FUNC(rsdProgramVertexInit);
    FunctionTable->vertex.setActive = NATIVE_FUNC(rsdProgramVertexSetActive);
    FunctionTable->vertex.destroy = NATIVE_FUNC(rsdProgramVertexDestroy);

    FunctionTable->fragment.init = NATIVE_FUNC(rsdProgramFragmentInit);
    FunctionTable->fragment.setActive = NATIVE_FUNC(rsdProgramFragmentSetActive);
    FunctionTable->fragment.destroy = NATIVE_FUNC(rsdProgramFragmentDestroy);

    FunctionTable->mesh.init = NATIVE_FUNC(rsdMeshInit);
    FunctionTable->mesh.draw = NATIVE_FUNC(rsdMeshDraw);
    FunctionTable->mesh.destroy = NATIVE_FUNC(rsdMeshDestroy);

    FunctionTable->path.initStatic = NATIVE_FUNC(rsdPathInitStatic);
    FunctionTable->path.initDynamic = NATIVE_FUNC(rsdPathInitDynamic);
    FunctionTable->path.draw = NATIVE_FUNC(rsdPathDraw);
    FunctionTable->path.destroy = NATIVE_FUNC(rsdPathDestroy);

    FunctionTable->sampler.init = rsdSamplerInit;
    FunctionTable->sampler.destroy = rsdSamplerDestroy;
    FunctionTable->sampler.updateCachedObject = rsdSamplerUpdateCachedObject;


    FunctionTable->framebuffer.init = NATIVE_FUNC(rsdFrameBufferInit);
    FunctionTable->framebuffer.setActive = NATIVE_FUNC(rsdFrameBufferSetActive);
    FunctionTable->framebuffer.destroy = NATIVE_FUNC(rsdFrameBufferDestroy);

    FunctionTable->scriptgroup.init = rsdScriptGroupInit;
    FunctionTable->scriptgroup.setInput = rsdScriptGroupSetInput;
    FunctionTable->scriptgroup.setOutput = rsdScriptGroupSetOutput;
    FunctionTable->scriptgroup.execute = rsdScriptGroupExecute;
    FunctionTable->scriptgroup.destroy = rsdScriptGroupDestroy;
    FunctionTable->scriptgroup.updateCachedObject = NULL;

    FunctionTable->type.init = rsdTypeInit; 
    FunctionTable->type.destroy = rsdTypeDestroy;
    FunctionTable->type.updateCachedObject = rsdTypeUpdateCachedObject;

    FunctionTable->element.init = rsdElementInit;
    FunctionTable->element.destroy = rsdElementDestroy;
    FunctionTable->element.updateCachedObject = rsdElementUpdateCachedObject;

    FunctionTable->finish = NULL;
};

extern const RsdCpuReference::CpuSymbol * rsdLookupRuntimeStub(Context * pContext, char const* name);

static RsdCpuReference::CpuScript * LookupScript(Context *, const Script *s) {
    return (RsdCpuReference::CpuScript *)s->mHal.drv;
}

extern "C" bool rsdHalInit(RsContext c, uint32_t version_major,
                           uint32_t version_minor) {
    Context *rsc = (Context*) c;
    setupFunctionTable(&rsc->mHal.funcs);

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
        rsc->mHal.drv = NULL;
        free(dc);
        return false;
    }

#ifndef RS_COMPATIBILITY_LIB
    // Set a callback for compiler setup here.
    if (false) {
        dc->mCpuRef->setSetupCompilerCallback(NULL);
    }

    // Set a callback for switching MemChunk's allocator here.
    // Note that the allocation function must return page-aligned memory, so
    // that it can be mprotected properly (i.e. code should be written and
    // later switched to read+execute only).
    if (false) {
        MemChunk::registerAllocFreeCallbacks(
                rsc->mHal.funcs.allocRuntimeMem,
                rsc->mHal.funcs.freeRuntimeMem);
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
    rsc->mHal.drv = NULL;
}

void* rsdAllocRuntimeMem(size_t size, uint32_t flags) {
    void* buffer = calloc(size, sizeof(char));
    return buffer;
}

void rsdFreeRuntimeMem(void* ptr) {
    free(ptr);
}
