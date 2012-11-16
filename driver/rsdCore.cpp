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
#include "rsdGL.h"
#include "rsdPath.h"
#include "rsdProgramStore.h"
#include "rsdProgramRaster.h"
#include "rsdProgramVertex.h"
#include "rsdProgramFragment.h"
#include "rsdMesh.h"
#include "rsdSampler.h"
#include "rsdScriptGroup.h"
#include "rsdFrameBuffer.h"

#include <malloc.h>
#include "rsContext.h"

#include <sys/types.h>
#include <sys/resource.h>
#include <sched.h>
#include <cutils/properties.h>
#include <sys/syscall.h>
#include <string.h>

using namespace android;
using namespace android::renderscript;

static void Shutdown(Context *rsc);
static void SetPriority(const Context *rsc, int32_t priority);

static RsdHalFunctions FunctionTable = {
    rsdGLInit,
    rsdGLShutdown,
    rsdGLSetSurface,
    rsdGLSwap,

    Shutdown,
    NULL,
    SetPriority,
    {
        rsdScriptInit,
        rsdInitIntrinsic,
        rsdScriptInvokeFunction,
        rsdScriptInvokeRoot,
        rsdScriptInvokeForEach,
        rsdScriptInvokeInit,
        rsdScriptInvokeFreeChildren,
        rsdScriptSetGlobalVar,
        rsdScriptSetGlobalVarWithElemDims,
        rsdScriptSetGlobalBind,
        rsdScriptSetGlobalObj,
        rsdScriptDestroy
    },

    {
        rsdAllocationInit,
        rsdAllocationDestroy,
        rsdAllocationResize,
        rsdAllocationSyncAll,
        rsdAllocationMarkDirty,
        rsdAllocationInitSurfaceTexture,
        rsdAllocationSetSurfaceTexture,
        rsdAllocationIoSend,
        rsdAllocationIoReceive,
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
        rsdAllocationGenerateMipmaps
    },


    {
        rsdProgramStoreInit,
        rsdProgramStoreSetActive,
        rsdProgramStoreDestroy
    },

    {
        rsdProgramRasterInit,
        rsdProgramRasterSetActive,
        rsdProgramRasterDestroy
    },

    {
        rsdProgramVertexInit,
        rsdProgramVertexSetActive,
        rsdProgramVertexDestroy
    },

    {
        rsdProgramFragmentInit,
        rsdProgramFragmentSetActive,
        rsdProgramFragmentDestroy
    },

    {
        rsdMeshInit,
        rsdMeshDraw,
        rsdMeshDestroy
    },

    {
        rsdPathInitStatic,
        rsdPathInitDynamic,
        rsdPathDraw,
        rsdPathDestroy
    },

    {
        rsdSamplerInit,
        rsdSamplerDestroy
    },

    {
        rsdFrameBufferInit,
        rsdFrameBufferSetActive,
        rsdFrameBufferDestroy
    },

    {
        rsdScriptGroupInit,
        rsdScriptGroupSetInput,
        rsdScriptGroupSetOutput,
        rsdScriptGroupExecute,
        rsdScriptGroupDestroy
    }


};

#if 0
<<<<<<< HEAD
pthread_key_t rsdgThreadTLSKey = 0;
uint32_t rsdgThreadTLSKeyCount = 0;
pthread_mutex_t rsdgInitMutex = PTHREAD_MUTEX_INITIALIZER;


static void * HelperThreadProc(void *vrsc) {
    Context *rsc = static_cast<Context *>(vrsc);
    RsdHal *dc = (RsdHal *)rsc->mHal.drv;


    uint32_t idx = (uint32_t)android_atomic_inc(&dc->mWorkers.mLaunchCount);

    //ALOGV("RS helperThread starting %p idx=%i", rsc, idx);

    dc->mWorkers.mLaunchSignals[idx].init();
    dc->mWorkers.mNativeThreadId[idx] = gettid();

    int status = pthread_setspecific(rsdgThreadTLSKey, &dc->mTlsStruct);
    if (status) {
        ALOGE("pthread_setspecific %i", status);
    }

#if 0
    typedef struct {uint64_t bits[1024 / 64]; } cpu_set_t;
    cpu_set_t cpuset;
    memset(&cpuset, 0, sizeof(cpuset));
    cpuset.bits[idx / 64] |= 1ULL << (idx % 64);
    int ret = syscall(241, rsc->mWorkers.mNativeThreadId[idx],
              sizeof(cpuset), &cpuset);
    ALOGE("SETAFFINITY ret = %i %s", ret, EGLUtils::strerror(ret));
#endif

    while (!dc->mExit) {
        dc->mWorkers.mLaunchSignals[idx].wait();
        if (dc->mWorkers.mLaunchCallback) {
           dc->mWorkers.mLaunchCallback(dc->mWorkers.mLaunchData, idx);
        }
        android_atomic_dec(&dc->mWorkers.mRunningCount);
        dc->mWorkers.mCompleteSignal.set();
    }

    //ALOGV("RS helperThread exited %p idx=%i", rsc, idx);
    return NULL;
}

void rsdLaunchThreads(Context *rsc, WorkerCallback_t cbk, void *data) {
    RsdHal *dc = (RsdHal *)rsc->mHal.drv;

    dc->mWorkers.mLaunchData = data;
    dc->mWorkers.mLaunchCallback = cbk;
    android_atomic_release_store(dc->mWorkers.mCount, &dc->mWorkers.mRunningCount);
    for (uint32_t ct = 0; ct < dc->mWorkers.mCount; ct++) {
        dc->mWorkers.mLaunchSignals[ct].set();
    }
    while (android_atomic_acquire_load(&dc->mWorkers.mRunningCount) != 0) {
        dc->mWorkers.mCompleteSignal.wait();
    }
=======
#else
extern const RsdCpuReference::CpuSymbol * rsdLookupRuntimeStub(Context * pContext, char const* name);

static RsdCpuReference::CpuScript * LookupScript(Context *, const Script *s) {
    return (RsdCpuReference::CpuScript *)s->mHal.drv;
#endif
//>>>>>>> 709a097... Separate CPU driver impl from reference driver.
}

extern "C" bool rsdHalInit(RsContext c, uint32_t version_major,
                           uint32_t version_minor) {
    Context *rsc = (Context*) c;
    rsc->mHal.funcs = FunctionTable;

    RsdHal *dc = (RsdHal *)calloc(1, sizeof(RsdHal));
    if (!dc) {
        ALOGE("Calloc for driver hal failed.");
        return false;
    }
    rsc->mHal.drv = dc;

#if 0
<<<<<<< HEAD
    pthread_mutex_lock(&rsdgInitMutex);
    if (!rsdgThreadTLSKeyCount) {
        int status = pthread_key_create(&rsdgThreadTLSKey, NULL);
        if (status) {
            ALOGE("Failed to init thread tls key.");
            pthread_mutex_unlock(&rsdgInitMutex);
            return false;
        }
    }
    rsdgThreadTLSKeyCount++;
    pthread_mutex_unlock(&rsdgInitMutex);

    dc->mTlsStruct.mContext = rsc;
    dc->mTlsStruct.mScript = NULL;
    int status = pthread_setspecific(rsdgThreadTLSKey, &dc->mTlsStruct);
    if (status) {
        ALOGE("pthread_setspecific %i", status);
    }


    int cpu = sysconf(_SC_NPROCESSORS_ONLN);
    if(rsc->props.mDebugMaxThreads) {
        cpu = rsc->props.mDebugMaxThreads;
    }
    if (cpu < 2) {
        cpu = 0;
    }
    ALOGV("%p Launching thread(s), CPUs %i", rsc, cpu);

    dc->mWorkers.mCount = (uint32_t)cpu;
    dc->mWorkers.mThreadId = (pthread_t *) calloc(dc->mWorkers.mCount, sizeof(pthread_t));
    dc->mWorkers.mNativeThreadId = (pid_t *) calloc(dc->mWorkers.mCount, sizeof(pid_t));
    dc->mWorkers.mLaunchSignals = new Signal[dc->mWorkers.mCount];
    dc->mWorkers.mLaunchCallback = NULL;

    dc->mWorkers.mCompleteSignal.init();

    android_atomic_release_store(dc->mWorkers.mCount, &dc->mWorkers.mRunningCount);
    android_atomic_release_store(0, &dc->mWorkers.mLaunchCount);

    pthread_attr_t threadAttr;
    status = pthread_attr_init(&threadAttr);
    if (status) {
        ALOGE("Failed to init thread attribute.");
=======
#else
    dc->mCpuRef = RsdCpuReference::create((Context *)c, version_major, version_minor,
                                          &rsdLookupRuntimeStub, &LookupScript);
    if (!dc->mCpuRef) {
        ALOGE("RsdCpuReference::create for driver hal failed.");
        free(dc);
#endif
//>>>>>>> 709a097... Separate CPU driver impl from reference driver.
        return false;
    }

    return true;
}


void SetPriority(const Context *rsc, int32_t priority) {
    RsdHal *dc = (RsdHal *)rsc->mHal.drv;

    dc->mCpuRef->setPriority(priority);

    if (dc->mHasGraphics) {
        rsdGLSetPriority(rsc, priority);
    }
}

void Shutdown(Context *rsc) {
    RsdHal *dc = (RsdHal *)rsc->mHal.drv;
    delete dc->mCpuRef;
    rsc->mHal.drv = NULL;
}

