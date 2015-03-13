#
# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

header:
summary: TODO
description:
# TODO move elsewhere?
 RenderScript is a high-performance runtime that provides
 compute operations at the native level. RenderScript code is compiled on devices
 at runtime to allow platform-independence as well.
 This reference documentation describes the RenderScript runtime APIs, which you
 can utilize to write RenderScript code in C99. The RenderScript compute header
 files are automatically included for you.

 To use RenderScript, you need to utilize the RenderScript runtime APIs documented here
 as well as the Android framework APIs for RenderScript.
 For documentation on the Android framework APIs, see the <a target="_parent" href="http://developer.android.com/reference/android/renderscript/package-summary.html">android.renderscript</a> package reference.
 For more information on how to develop with RenderScript and how the runtime and
 Android framework APIs interact, see the <a target="_parent" href="http://developer.android.com/guide/topics/renderscript/index.html">RenderScript developer guide</a>
 and the <a target="_parent" href="http://developer.android.com/resources/samples/RenderScript/index.html">RenderScript samples</a>.
include:
 #define RS_KERNEL __attribute__((kernel))

 #include "rs_types.rsh"
 #include "rs_allocation.rsh"
 #include "rs_atomic.rsh"
 #include "rs_core_math.rsh"
 #include "rs_debug.rsh"
 #include "rs_element.rsh"
 #include "rs_math.rsh"
 #include "rs_matrix.rsh"
 #include "rs_object.rsh"
 #include "rs_quaternion.rsh"
 #include "rs_sampler.rsh"
 #include "rs_time.rsh"
end:

type: rs_for_each_strategy_t
enum: rs_for_each_strategy
value: RS_FOR_EACH_STRATEGY_SERIAL = 0
value: RS_FOR_EACH_STRATEGY_DONT_CARE = 1
value: RS_FOR_EACH_STRATEGY_DST_LINEAR = 2
value: RS_FOR_EACH_STRATEGY_TILE_SMALL = 3
value: RS_FOR_EACH_STRATEGY_TILE_MEDIUM = 4
value: RS_FOR_EACH_STRATEGY_TILE_LARGE = 5
summary: Launch order hint for rsForEach calls
description:
 Launch order hint for rsForEach calls.  This provides a hint to the system to
 determine in which order the root function of the target is called with each
 cell of the allocation.

 This is a hint and implementations may not obey the order.
end:

type: rs_kernel_context
version: 23
simple: const struct rs_kernel_context_t *
summary: Opaque handle to RenderScript kernel invocation context
description:
 TODO
end:

type: rs_script_call_t
struct: rs_script_call
field: rs_for_each_strategy_t strategy
field: uint32_t xStart
field: uint32_t xEnd
field: uint32_t yStart
field: uint32_t yEnd
field: uint32_t zStart
field: uint32_t zEnd
field: uint32_t arrayStart
field: uint32_t arrayEnd
summary: Provides extra information to a rsForEach call
description:
 Structure to provide extra information to a rsForEach call.  Primarly used to
 restrict the call to a subset of cells in the allocation.
end:

function: rsForEach
version: 9 13
ret: void
arg: rs_script script, "The target script to call"
arg: rs_allocation input, "The allocation to source data from"
arg: rs_allocation output, "the allocation to write date into"
arg: const void* usrData, "The user defined params to pass to the root script.  May be NULL."
arg: const rs_script_call_t* sc, "Extra control infomation used to select a sub-region of the allocation to be processed or suggest a walking strategy.  May be NULL."
summary:
description:
 Make a script to script call to launch work. One of the input or output is
 required to be a valid object. The input and output must be of the same
 dimensions.
test: none
end:

function: rsForEach
version: 9 13
ret: void
arg: rs_script script
arg: rs_allocation input
arg: rs_allocation output
arg: const void* usrData
test: none
end:

function: rsForEach
version: 14 20
ret: void
arg: rs_script script
arg: rs_allocation input
arg: rs_allocation output
arg: const void* usrData
arg: size_t usrDataLen, "The size of the userData structure.  This will be used to perform a shallow copy of the data if necessary."
arg: const rs_script_call_t* sc
test: none
end:

function: rsForEach
version: 14 20
ret: void
arg: rs_script script
arg: rs_allocation input
arg: rs_allocation output
arg: const void* usrData
arg: size_t usrDataLen
test: none
end:

function: rsForEach
version: 14
ret: void
arg: rs_script script
arg: rs_allocation input
arg: rs_allocation output
test: none
end:

function: rsSendToClient
ret: bool
arg: int cmdID
summary:
description:
 Send a message back to the client.  Will not block and returns true
 if the message was sendable and false if the fifo was full.
 A message ID is required.  Data payload is optional.
test: none
end:

function: rsSendToClient
ret: bool
arg: int cmdID
arg: const void* data
arg: uint len
test: none
end:

function: rsSendToClientBlocking
ret: void
arg: int cmdID
summary:
description:
 Send a message back to the client, blocking until the message is queued.
 A message ID is required.  Data payload is optional.
test: none
end:

function: rsSendToClientBlocking
ret: void
arg: int cmdID
arg: const void* data
arg: uint len
test: none
end:

function: rsGetDimX
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary:
description:
 Return X dimension of kernel launch described by the specified launch context.
test: none
end:

function: rsGetDimY
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary:
description:
 Return Y dimension of kernel launch described by the specified launch context.

 Returns 0 if Y dimension is not present.
test: none
end:

function: rsGetDimZ
version: 23
ret: uint32_t
arg: rs_kernel_context ctxt
summary:
description:
 Return Z dimension of kernel launch described by the specified launch context.

 Returns 0 if Z dimension is not present.
test: none
end:
