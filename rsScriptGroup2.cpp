#include "rsScriptGroup2.h"

#include "rsClosure.h"
#include "rsContext.h"

namespace android {
namespace renderscript {

ScriptGroup2::ScriptGroup2(Context* rsc, const char* cacheDir, Closure** closures,
                           size_t numClosures) :
    ScriptGroupBase(rsc), mClosures(closures, closures + numClosures),
    mCacheDir(cacheDir) {
    for (auto c : mClosures) {
        c->incSysRef();
    }
}

ScriptGroup2::~ScriptGroup2() {
    if (mRSC->mHal.funcs.scriptgroup.destroy) {
        mRSC->mHal.funcs.scriptgroup.destroy(mRSC, this);
    }

    for (auto c : mClosures) {
        c->decSysRef();
    }
}

void ScriptGroup2::execute(Context* rsc) {
    if (rsc->mHal.funcs.scriptgroup.execute) {
        rsc->mHal.funcs.scriptgroup.execute(rsc, this);
    }
}

RsScriptGroup2 rsi_ScriptGroup2Create(Context* rsc, const char* cacheDir,
                                      size_t cacheDirLength,
                                      RsClosure* closures, size_t numClosures) {
    ScriptGroup2* group = new ScriptGroup2(rsc, cacheDir, (Closure**)closures, numClosures);

    // Create a device-specific implementation by calling the device driver
    if (rsc->mHal.funcs.scriptgroup.init) {
        rsc->mHal.funcs.scriptgroup.init(rsc, group);
    }

    group->incUserRef();

    return group;
}

}  // namespace renderscript
}  // namespace android
