#ifndef ANDROID_RENDERSCRIPT_SCRIPTGROUP2_H_
#define ANDROID_RENDERSCRIPT_SCRIPTGROUP2_H_

#include <list>

#include "rsDefines.h"
#include "rsObjectBase.h"

using std::list;

namespace android {
namespace renderscript {

class Closure;
class Context;

class ScriptGroup2 : public ObjectBase {
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
      ObjectBase(rsc), mClosures(closures, closures + numClosures) {}
  ~ScriptGroup2() {}

  virtual void serialize(Context *rsc, OStream *stream) const {}

  virtual RsA3DClassID getClassId() const {
    return RS_A3D_CLASS_ID_SCRIPT_GROUP2;
  }

  void execute(Context* rsc);

  list<Closure*> mClosures;
};

}  // namespace renderscript
}  // namespace android

#endif  // ANDROID_RENDERSCRIPT_SCRIPTGROUP2_H_
