#include "rsCpuScriptGroup2.h"

#include "bcc/BCCContext.h"
#include "bcc/Renderscript/RSCompilerDriver.h"
#include "bcc/Renderscript/RSExecutable.h"
#include "bcc/Source.h"
#include "cpu_ref/rsCpuCore.h"
#include "rsClosure.h"
#include "rsContext.h"
#include "rsCpuCore.h"
#include "rsCpuScript.h"
#include "rsScript.h"
#include "rsScriptGroup2.h"
#include "rsScriptIntrinsic.h"

namespace android {
namespace renderscript {

namespace {

#ifdef __LP64__
const char* SysLibPath = "/system/lib64";
#else
const char* SysLibPath = "/system/lib";
#endif

const char* CacheDir =
    "/data/data/com.android.rs.image/cache/com.android.renderscript.cache";
const size_t DefaultKernelArgCount = 2;

void groupRoot(const RsExpandKernelParams *kparams, uint32_t xstart,
               uint32_t xend, uint32_t outstep) {
  const list<CPUClosure*>& closures = *(list<CPUClosure*>*)kparams->usr;
  RsExpandKernelParams *mutable_kparams = (RsExpandKernelParams *)kparams;
  const void **oldIns  = kparams->ins;
  uint32_t *oldStrides = kparams->inEStrides;

  std::vector<const void*> ins(DefaultKernelArgCount);
  std::vector<uint32_t> strides(DefaultKernelArgCount);

  for (CPUClosure* cpuClosure : closures) {
    const Closure* closure = cpuClosure->mClosure;

    auto in_iter = ins.begin();
    auto stride_iter = strides.begin();

    for (const auto& arg : closure->mArgs) {
      const Allocation* a = (const Allocation*)arg;
      const uint32_t eStride = a->mHal.state.elementSizeBytes;
      const uint8_t* ptr = (uint8_t*)(a->mHal.drvState.lod[0].mallocPtr) +
          eStride * xstart;
      if (kparams->dimY > 1) {
        ptr += a->mHal.drvState.lod[0].stride * kparams->y;
      }
      *in_iter++ = ptr;
      *stride_iter++ = eStride;
    }

    mutable_kparams->ins = &ins[0];
    mutable_kparams->inEStrides = &strides[0];

    const Allocation* out = closure->mReturnValue;
    const uint32_t ostep = out->mHal.state.elementSizeBytes;
    const uint8_t* ptr = (uint8_t *)(out->mHal.drvState.lod[0].mallocPtr) +
           ostep * xstart;
    if (kparams->dimY > 1) {
      ptr += out->mHal.drvState.lod[0].stride * kparams->y;
    }

    mutable_kparams->out = (void*)ptr;

    mutable_kparams->usr = cpuClosure->mUsrPtr;

    cpuClosure->mFunc(kparams, xstart, xend, ostep);
  }

  mutable_kparams->ins        = oldIns;
  mutable_kparams->inEStrides = oldStrides;
  mutable_kparams->usr        = &closures;
}

}  // namespace

bool Batch::conflict(CPUClosure* closure) const {
  for (const auto &p : closure->mClosure->mGlobalDeps) {
    const Closure* dep = p.first;
    for (CPUClosure* c : mClosures) {
      if (c->mClosure == dep) {
        ALOGV("ScriptGroup2: closure %p conflicting with closure %p via its global", closure, dep);
        return true;
      }
    }
  }
  for (const auto &p : closure->mClosure->mArgDeps) {
    const Closure* dep = p.first;
    for (CPUClosure* c : mClosures) {
      if (c->mClosure == dep) {
        for (const auto &p1 : *p.second) {
          if (p1.second != nullptr) {
            ALOGV("ScriptGroup2: closure %p conflicting with closure %p via its arg", closure, dep);
            return true;
          }
        }
      }
    }
  }
  return false;
}

CpuScriptGroup2Impl::CpuScriptGroup2Impl(RsdCpuReferenceImpl *cpuRefImpl,
                                         const ScriptGroup2 *group) :
  mCpuRefImpl(cpuRefImpl), mGroup(group) {
  Batch* batch = new Batch(this);
  for (Closure* closure: group->mClosures) {
    const ScriptKernelID* kernelID = closure->mKernelID;
    RsdCpuScriptImpl* si =
        (RsdCpuScriptImpl *)mCpuRefImpl->lookupScript(kernelID->mScript);

    MTLaunchStruct mtls;
    si->forEachKernelSetup(kernelID->mSlot, &mtls);
    // TODO: Is mtls.fep.usrLen ever used?
    CPUClosure* cc = new CPUClosure(closure, si, (ExpandFuncTy)mtls.kernel,
                                    mtls.fep.usr, mtls.fep.usrLen);
    if (batch->conflict(cc)) {
      mBatches.push_back(batch);
      batch = new Batch(this);
    }
    batch->mClosures.push_back(cc);
  }
  mBatches.push_back(batch);

  for (Batch* batch : mBatches) {
    batch->tryToCreateFusedKernel(CacheDir);
  }
}

void Batch::tryToCreateFusedKernel(const char *cacheDir) {
  if (mClosures.size() < 2) {
    ALOGV("Compiler kernel fusion skipped due to only one or zero kernel in"
          " a script group batch.");
    return;
  }

  std::vector<const bcc::Source*> sources;
  std::vector<int> slots;

  for(CPUClosure* cpuClosure : mClosures) {
    const Closure* closure = cpuClosure->mClosure;
    const ScriptKernelID* kernelID = closure->mKernelID;
    const Script* script = kernelID->mScript;

    if (script->isIntrinsic()) {
      return;
    }

    const RsdCpuScriptImpl *cpuScript =
        (const RsdCpuScriptImpl*)script->mHal.drv;
    sources.push_back(cpuScript->getSource());
    slots.push_back(kernelID->mSlot);
  }

  // bcc::BCCContext context;
  bcc::RSCompilerDriver driver;

  bcc::SymbolResolverProxy resolver;
  bcc::CompilerRTSymbolResolver compilerRuntime;
  bcc::LookupFunctionSymbolResolver<void *> RSRuntime;

  RSRuntime.setLookupFunction(RsdCpuScriptImpl::lookupRuntimeStub);
  RSRuntime.setContext(mClosures.front()->mClosure->mKernelID->mScript);
  resolver.chainResolver(compilerRuntime);
  resolver.chainResolver(RSRuntime);

  std::string runtimePath = std::string(SysLibPath) + "/libclcore.bc";

  mExecutable = driver.buildScriptGroup(//context,
      ((bcc::Source*)sources.front())->getContext(),
      cacheDir, runtimePath.c_str(),
      sources, slots, resolver);
}

void CpuScriptGroup2Impl::execute() {
  for (auto batch : mBatches) {
    batch->setGlobalsForBatch();
    batch->run();
  }
}

void Batch::setGlobalsForBatch() {
  for (CPUClosure* cpuClosure : mClosures) {
    const Closure* closure = cpuClosure->mClosure;
    const ScriptKernelID* kernelID = closure->mKernelID;
    Script* s = kernelID->mScript;
    for (const auto& p : closure->mGlobals) {
      const void* value = p.second.first;
      int size = p.second.second;
      // We use -1 size to indicate an ObjectBase rather than a primitive type
      if (size < 0) {
        s->setVarObj(p.first->mSlot, (ObjectBase*)value);
      } else {
        s->setVar(p.first->mSlot, (const void*)&value, size);
      }
    }
  }
}

void Batch::run() {
  if (mExecutable != nullptr) {
    MTLaunchStruct mtls;
    const CPUClosure* firstCpuClosure = mClosures.front();
    const CPUClosure* lastCpuClosure = mClosures.back();

    firstCpuClosure->mSi->forEachMtlsSetup(
        (const Allocation**)&firstCpuClosure->mClosure->mArgs[0],
        firstCpuClosure->mClosure->mArgs.size(),
        lastCpuClosure->mClosure->mReturnValue,
        nullptr, 0, nullptr, &mtls);

    mtls.script = nullptr;
    mtls.fep.usr = nullptr;
    mtls.kernel = reinterpret_cast<ForEachFunc_t>(
        mExecutable->getExportForeachFuncAddrs()[0]);

    mGroup->getCpuRefImpl()->launchThreads(
        (const Allocation**)&firstCpuClosure->mClosure->mArgs[0],
        firstCpuClosure->mClosure->mArgs.size(),
        lastCpuClosure->mClosure->mReturnValue,
        nullptr, &mtls);

    return;
  }

  for (CPUClosure* cpuClosure : mClosures) {
    const Closure* closure = cpuClosure->mClosure;
    const ScriptKernelID* kernelID = closure->mKernelID;
    cpuClosure->mSi->preLaunch(kernelID->mSlot,
                               (const Allocation**)&closure->mArgs[0],
                               closure->mArgs.size(), closure->mReturnValue,
                               cpuClosure->mUsrPtr, cpuClosure->mUsrSize,
                               nullptr);
  }

  const CPUClosure* cpuClosure = mClosures.front();
  const Closure* closure = cpuClosure->mClosure;
  MTLaunchStruct mtls;

  cpuClosure->mSi->forEachMtlsSetup((const Allocation**)&closure->mArgs[0],
                                    closure->mArgs.size(),
                                    closure->mReturnValue,
                                    nullptr, 0, nullptr, &mtls);

  mtls.script = nullptr;
  mtls.kernel = (void (*)())&groupRoot;
  mtls.fep.usr = &mClosures;

  mGroup->getCpuRefImpl()->launchThreads(nullptr, 0, nullptr, nullptr, &mtls);

  for (CPUClosure* cpuClosure : mClosures) {
    const Closure* closure = cpuClosure->mClosure;
    const ScriptKernelID* kernelID = closure->mKernelID;
    cpuClosure->mSi->postLaunch(kernelID->mSlot,
                                (const Allocation**)&closure->mArgs[0],
                                closure->mArgs.size(), closure->mReturnValue,
                                nullptr, 0, nullptr);
  }
}

}  // namespace renderscript
}  // namespace android
