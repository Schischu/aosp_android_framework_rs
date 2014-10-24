#ifndef ANDROID_RENDERSCRIPT_SCRIPTGROUP2_H_
#define ANDROID_RENDERSCRIPT_SCRIPTGROUP2_H_

#include <list>

#include "rsDefines.h"

using std::list;

namespace android {
namespace renderscript {

class Closure;
class Context;

class ScriptGroup2 {
 public:
  struct Hal {
    void * drv;

    struct DriverInfo {
    };
    DriverInfo info;
  };
  Hal mHal;

  /*
    TODO:
    Inputs and outputs are set and retrieved in Java runtime.
    They are opaque in the C++ runtime.
    For better compiler optimizations (of a script group), we need to include
    input and output information in the C++ runtime.
   */
  ScriptGroup2(Context* rsc, Closure** closures, size_t numClosures) :
      mClosures(closures, closures + numClosures) {}
  ~ScriptGroup2() {}

  void execute(Context* rsc);

  list<Closure*> mClosures;
};

RsScriptGroup2 rsi_ScriptGroup2Create(Context*, RsClosure*, size_t);
void rsi_ScriptGroup2Execute(Context*, RsScriptGroup2);

}  // namespace renderscript
}  // namespace android

#endif  // ANDROID_RENDERSCRIPT_SCRIPTGROUP2_H_
