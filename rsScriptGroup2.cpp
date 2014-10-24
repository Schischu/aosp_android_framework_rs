#include "rsScriptGroup2.h"

#include "cpu_ref/rsCpuCore.h"
#include "rsClosure.h"
#include "rsContext.h"
#include "rsScript.h"

namespace android {
namespace renderscript {

void ScriptGroup2::execute(Context* rsc) {
  if (rsc->mHal.funcs.scriptgroup2.execute) {
    rsc->mHal.funcs.scriptgroup2.execute(rsc, this);
  }
}

RsScriptGroup2 rsi_ScriptGroup2Create(Context* rsc, RsClosure* closures,
                                      size_t numClosures) {
  ScriptGroup2* group = new ScriptGroup2(rsc, (Closure**)closures, numClosures);

  if (rsc->mHal.funcs.scriptgroup2.init) {
    rsc->mHal.funcs.scriptgroup2.init(rsc, group);
  }

  // TODO: reference counting the script group?

  return group;
}

void rsi_ScriptGroup2Execute(Context* rsc, RsScriptGroup2 sg) {
  ScriptGroup2* group = (ScriptGroup2*)sg;
  group->execute(rsc);
}

}  // namespace renderscript
}  // namespace android
