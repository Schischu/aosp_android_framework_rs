#ifndef CPU_REF_CPUSCRIPTGROUP2IMPL_H_
#define CPU_REF_CPUSCRIPTGROUP2IMPL_H_

#include <list>

#include "rsd_cpu.h"

using std::list;

namespace bcc {
class RSExecutable;
} // namespace bcc

using bcc::RSExecutable;

namespace android {
namespace renderscript {

class Closure;
class RsdCpuScriptImpl;
class RsdCpuReferenceImpl;
class ScriptGroup2;

struct RsExpandKernelParams;

typedef void (*ExpandFuncTy)(const RsExpandKernelParams*, uint32_t, uint32_t,
                             uint32_t);

class CPUClosure {
 public:
  CPUClosure(const Closure* closure, RsdCpuScriptImpl* si, ExpandFuncTy func,
             const void* usrPtr, const size_t usrSize) :
      mClosure(closure), mSi(si), mFunc(func), mUsrPtr(usrPtr),
      mUsrSize(usrSize) {}

  // It's important to do forwarding here than inheritance for unbound value
  // binding to work.
  const Closure* mClosure;
  RsdCpuScriptImpl* mSi;
  const ExpandFuncTy mFunc;
  const void* mUsrPtr;
  const size_t mUsrSize;
};

class CpuScriptGroup2Impl;

class Batch {
 public:
  Batch(CpuScriptGroup2Impl* group) : mGroup(group), mExecutable(nullptr) {}

  ~Batch() {
    for (CPUClosure* c : mClosures) {
      delete c;
    }
  }

  // Returns true if closure depends on any closure in this batch for a glboal
  // variable
  bool conflict(CPUClosure* closure) const;

  void tryToCreateFusedKernel(const char* cacheDir);
  void setGlobalsForBatch();
  void run();

  CpuScriptGroup2Impl* mGroup;
  RSExecutable* mExecutable;
  list<CPUClosure*> mClosures;
};

class CpuScriptGroup2Impl : public RsdCpuReference::CpuScriptGroup2 {
 public:
  CpuScriptGroup2Impl(RsdCpuReferenceImpl *cpuRefImpl, const ScriptGroup2* group);
  virtual ~CpuScriptGroup2Impl() {
    for (Batch* batch : mBatches) {
      delete batch;
    }
  }

  bool init();
  virtual void execute();

  RsdCpuReferenceImpl* getCpuRefImpl() const { return mCpuRefImpl; }

 private:
  RsdCpuReferenceImpl* mCpuRefImpl;
  const ScriptGroup2* mGroup;
  list<Batch*> mBatches;
};

}  // namespace renderscript
}  // namespace android

#endif  // CPU_REF_CPUSCRIPTGROUP2IMPL_H_
