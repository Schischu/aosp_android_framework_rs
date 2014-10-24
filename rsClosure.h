#ifndef ANDROID_RENDERSCRIPT_CLOSURE_H_
#define ANDROID_RENDERSCRIPT_CLOSURE_H_

#include <map>
#include <set>
#include <vector>

#include "rsDefines.h"

namespace android {
namespace renderscript {

using std::map;
using std::pair;
using std::set;
using std::vector;

class Allocation;
class Context;
class ScriptFieldID;
class ScriptKernelID;
class Type;

class Closure {
 public:
  Closure(Context* context,
          const ScriptKernelID* kernelID,
          Allocation* returnValue,
          const int numValues,
          const ScriptFieldID** fieldIDs,
          const void** values,  // Allocations or primitive (numeric) types
          const int* sizes,   // size for data type. -1 indicates an allocation.
          const Closure** depClosures,
          const ScriptFieldID** depFieldIDs);
  ~Closure();

  void eval();

  void setArg(const int index, const void* value, const int size);
  void setGlobal(const ScriptFieldID* fieldID, const void* value,
                 const int size);

  Context* mContext;
  const ScriptKernelID* mKernelID;

  // Values referrenced in arguments and globals cannot be futures. They must be
  // either a known value or unbound value.
  // For now, all arguments should be Allocations.
  vector<const void*> mArgs;

  // A global could be allocation or any primitive data type.
  map<const ScriptFieldID*, pair<const void*, int>> mGlobals;

  Allocation* mReturnValue;

  // All the other closures that this closure depends on
  set<const Closure*> mDependences;

  // All the other closures which this closure depends on for one of its
  // arguments, and the fields which it depends on.
  map<const Closure*, map<int, const ScriptFieldID*>*> mArgDeps;

  // All the other closures that this closure depends on for one of its fields,
  // and the fields that it depends on.
  map<const Closure*, map<const ScriptFieldID*, const ScriptFieldID*>*> mGlobalDeps;
};

RsClosure rsi_ClosureCreate(Context* context, RsScriptKernelID kernelID,
                            RsAllocation returnValue,
                            RsScriptFieldID* fieldIDs, size_t fieldIDs_length,
                            uintptr_t* values, size_t values_length,
                            size_t* sizes, size_t sizes_length,
                            RsClosure* depClosures, size_t depClosures_length,
                            RsScriptFieldID* depFieldIDs,
                            size_t depFieldIDs_length);

void rsi_ClosureEval(Context*, RsClosure closure);

}  // namespace renderscript
}  // namespace android

#endif  // ANDROID_RENDERSCRIPT_CLOSURE_H_
