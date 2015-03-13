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
summary: Utility debugging routines
description:
 Routines intended to be used during application developement.  These should
 not be used in shipping applications.  All print a string and value pair to
 the standard log.
include:
 #define RS_DEBUG(a) rsDebug(#a, a)
 #define RS_DEBUG_MARKER rsDebug(__FILE__, __LINE__)
end:

function: rsDebug
t: i32, u32, i64, u64, f64
ret: void
arg: const char* message
arg: #1 a
summary:
description:
 Debug function.  Prints a string and value to the log.
test: none
end:

function: rsDebug
version: 17
w: 2, 3, 4
# TODO We're not doing it for f64?
t: i32, u32, i64, u64
ret: void
arg: const char* message
arg: #2#1 a
test: none
end:

function: rsDebug
w: 1, 2, 3, 4
ret: void
arg: const char* message
arg: float#1 a
test: none
end:

function: rsDebug
version: 17
w: 1, 2, 3, 4
t: i8, u8, i16, u16
ret: void
arg: const char* message
arg: #2#1 a
test: none
end:

function: rsDebug
ret: void
arg: const char* message
arg: float a
arg: float b
test: none
end:

function: rsDebug
ret: void
arg: const char* message
arg: float a
arg: float b
arg: float c
test: none
end:

function: rsDebug
ret: void
arg: const char* message
arg: float a
arg: float b
arg: float c
arg: float d
test: none
end:

function: rsDebug
ret: void
arg: const char* message
arg: long long a
test: none
end:

function: rsDebug
ret: void
arg: const char* message
arg: unsigned long long a
test: none
end:

function: rsDebug
ret: void
arg: const char* message
arg: const void* a
test: none
end:

function: rsDebug
ret: void
arg: const char* message
arg: const rs_matrix4x4* a
test: none
end:

function: rsDebug
ret: void
arg: const char* message
arg: const rs_matrix3x3* a
test: none
end:

function: rsDebug
ret: void
arg: const char* message
arg: const rs_matrix2x2* a
test: none
end:

#define RS_DEBUG(a) rsDebug(#a, a)
#define RS_DEBUG_MARKER rsDebug(__FILE__, __LINE__)
