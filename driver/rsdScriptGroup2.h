#ifndef RSD_SCRIPT_GROUP2_H
#define RSD_SCRIPT_GROUP2_H

#include <rs_hal.h>

using android::renderscript::Context;
using android::renderscript::ScriptGroup2;

bool rsdScriptGroup2Init(const Context*, ScriptGroup2*);
void rsdScriptGroup2Execute(const Context*, const ScriptGroup2*);

#endif  // RSD_SCRIPT_GROUP2_H
