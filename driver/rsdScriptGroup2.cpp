#include "rsdScriptGroup2.h"

#include "rsContext.h"
#include "rsdCore.h"
#include "../cpu_ref/rsd_cpu.h"
#include "rsScriptGroup2.h"

using android::renderscript::RsdCpuReference;

bool rsdScriptGroup2Init(const Context* context, ScriptGroup2* group) {
  RsdHal *dc = (RsdHal *)context->mHal.drv;

  group->mHal.drv = dc->mCpuRef->createScriptGroup2(group);
  return group->mHal.drv != nullptr;
}

void rsdScriptGroup2Execute(const Context* context,
                            const ScriptGroup2* group) {
    RsdCpuReference::CpuScriptGroup2* sgi =
        (RsdCpuReference::CpuScriptGroup2*)group->mHal.drv;
    sgi->execute();
}
