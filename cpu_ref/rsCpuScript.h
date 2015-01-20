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

#ifndef RSD_BCC_H
#define RSD_BCC_H

#include <rs_hal.h>
#include <rsRuntime.h>

#ifndef RS_COMPATIBILITY_LIB
#include <vector>
#include <utility>
#endif

#include "rsCpuCore.h"

namespace bcc {
    class BCCContext;
    class RSCompilerDriver;
}

namespace bcinfo {
    class MetadataExtractor;
}

namespace android {
namespace renderscript {

class ScriptExecutable {
 public:
  ScriptExecutable(Context* RSContext,
                   size_t varCount, void* fieldAddress[], bool fieldIsObject[],
                   size_t funcCount, InvokeFunc_t invokeFunctions[],
                   size_t forEachCount, ForEachFunc_t forEachFunctions[],
                   uint32_t forEachSignatures[]) :
      mExportedVariableCount(varCount), mExportedFunctionCount(funcCount),
      mExportedForEachCount(forEachCount),
      mFieldAddress(fieldAddress), mFieldIsObject(fieldIsObject),
      mInvokeFunctions(invokeFunctions), mForEachFunctions(forEachFunctions),
      mForEachSignatures(forEachSignatures),
      mRS(RSContext) {}

  ~ScriptExecutable() {
      if (mFieldIsObject) {
          for (size_t i = 0; i < mExportedVariableCount; ++i) {
              if (mFieldIsObject[i]) {
                  if (mFieldAddress[i] != nullptr) {
                      rs_object_base *obj_addr =
                          reinterpret_cast<rs_object_base *>((mFieldAddress.get())[i]);
                      rsrClearObject(mRS, obj_addr);
                  }
              }
          }
      }
  }

  static ScriptExecutable*
  createFromSharedObject(Context* RSContext, void* sharedObj);

  size_t getExportedVariableCount() const { return mExportedVariableCount; }
  size_t getExportedFunctionCount() const { return mExportedFunctionCount; }
  size_t getExportedForEachCount() const { return mExportedForEachCount; }

  void** getFieldAddress() const { return mFieldAddress.get(); }
  bool* getFieldIsObject() const { return mFieldIsObject.get(); }
  InvokeFunc_t* getInvokeFunctions() const { return mInvokeFunctions.get(); }
  ForEachFunc_t* getForEachFunctions() const { return mForEachFunctions.get(); }
  uint32_t* getForEachSignatures() const { return mForEachSignatures.get(); }

 private:
  const size_t mExportedVariableCount;
  const size_t mExportedFunctionCount;
  const size_t mExportedForEachCount;

  std::unique_ptr<void*[]> mFieldAddress;
  std::unique_ptr<bool[]> mFieldIsObject;
  std::unique_ptr<InvokeFunc_t[]> mInvokeFunctions;
  std::unique_ptr<ForEachFunc_t[]> mForEachFunctions;
  std::unique_ptr<uint32_t[]> mForEachSignatures;

  Context* mRS;
};

class RsdCpuScriptImpl : public RsdCpuReferenceImpl::CpuScript {
public:
    typedef void (*outer_foreach_t)(
        const RsExpandKernelParams *,
        uint32_t x1, uint32_t x2,
        uint32_t outstep);

    typedef void (* InvokeFunc_t)(void);
    typedef void (* ForEachFunc_t)(void);
    typedef int (* RootFunc_t)(void);
#ifdef RS_COMPATIBILITY_LIB
    typedef void (*WorkerCallback_t)(void *usr, uint32_t idx);
#endif

    bool init(char const *resName, char const *cacheDir,
              uint8_t const *bitcode, size_t bitcodeSize, uint32_t flags,
              char const *bccPluginName = nullptr);
    virtual void populateScript(Script *);

    virtual void invokeFunction(uint32_t slot, const void *params, size_t paramLength);
    virtual int invokeRoot();
    virtual void preLaunch(uint32_t slot, const Allocation ** ains,
                           uint32_t inLen, Allocation * aout, const void * usr,
                           uint32_t usrLen, const RsScriptCall *sc);
    virtual void postLaunch(uint32_t slot, const Allocation ** ains,
                            uint32_t inLen, Allocation * aout,
                            const void * usr, uint32_t usrLen,
                            const RsScriptCall *sc);

    virtual void invokeForEach(uint32_t slot,
                               const Allocation ** ains,
                               uint32_t inLen,
                               Allocation* aout,
                               const void* usr,
                               uint32_t usrLen,
                               const RsScriptCall* sc);

    virtual void invokeInit();
    virtual void invokeFreeChildren();

    virtual void setGlobalVar(uint32_t slot, const void *data, size_t dataLength);
    virtual void getGlobalVar(uint32_t slot, void *data, size_t dataLength);
    virtual void setGlobalVarWithElemDims(uint32_t slot, const void *data, size_t dataLength,
                                  const Element *e, const uint32_t *dims, size_t dimLength);
    virtual void setGlobalBind(uint32_t slot, Allocation *data);
    virtual void setGlobalObj(uint32_t slot, ObjectBase *data);


    virtual ~RsdCpuScriptImpl();
    RsdCpuScriptImpl(RsdCpuReferenceImpl *ctx, const Script *s);

    const Script * getScript() {return mScript;}

    void forEachMtlsSetup(const Allocation ** ains, uint32_t inLen,
                          Allocation * aout, const void * usr, uint32_t usrLen,
                          const RsScriptCall *sc, MTLaunchStruct *mtls);

    virtual void forEachKernelSetup(uint32_t slot, MTLaunchStruct *mtls);


    const RsdCpuReference::CpuSymbol * lookupSymbolMath(const char *sym);
    static void * lookupRuntimeStub(void* pContext, char const* name);

    virtual Allocation * getAllocationForPointer(const void *ptr) const;
    bool storeRSInfoFromSO();

protected:
    RsdCpuReferenceImpl *mCtx;
    const Script *mScript;
    void *mScriptSO;

#ifndef RS_COMPATIBILITY_LIB
    // Returns the path to the core library we'll use.
    const char* findCoreLib(const bcinfo::MetadataExtractor& bitCodeMetaData, const char* bitcode,
                            size_t bitcodeSize);

    bcc::RSCompilerDriver *mCompilerDriver;
#endif

    RootFunc_t mRoot;
    RootFunc_t mRootExpand;
    InvokeFunc_t mInit;
    InvokeFunc_t mFreeChildren;
    std::unique_ptr<ScriptExecutable> mScriptExec;

    Allocation **mBoundAllocs;
    void * mIntrinsicData;
    bool mIsThreadable;
};

Allocation * rsdScriptGetAllocationForPointer(
                        const Context *dc,
                        const Script *script,
                        const void *);



}

}

#endif
