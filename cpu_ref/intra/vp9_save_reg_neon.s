@ This file was created from a .asm file
@  using the ads2gas.pl script.
	.equ DO1STROUNDING, 0
@
@  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
@
@  Use of this source code is governed by a BSD-style license
@  that can be found in the LICENSE file in the root of the source
@  tree. An additional intellectual property rights grant can be found
@  in the file PATENTS.  All contributing project authors may
@  be found in the AUTHORS file in the root of the source tree.
@
@  Copyright (c) 2014 The Android Open Source Project
@
@  Licensed under the Apache License, Version 2.0 (the "License");
@  you may not use this file except in compliance with the License.
@  You may obtain a copy of the License at
@
@      http://www.apache.org/licenses/LICENSE-2.0
@
@  Unless required by applicable law or agreed to in writing, software
@  distributed under the License is distributed on an "AS IS" BASIS,
@  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@  See the License for the specific language governing permissions and
@  limitations under the License.

    .global vp9_push_neon
	.type vp9_push_neon, function
    .global vp9_pop_neon
	.type vp9_pop_neon, function

   .arm
   .eabi_attribute 24, 1 @Tag_ABI_align_needed
   .eabi_attribute 25, 1 @Tag_ABI_align_preserved

.text
.p2align 2

_vp9_push_neon:
	vp9_push_neon: @ PROC
    vst1.i64            {d8, d9, d10, d11}, [r0]!
    vst1.i64            {d12, d13, d14, d15}, [r0]!
    bx              lr

	.size vp9_push_neon, .-vp9_push_neon    @ ENDP

_vp9_pop_neon:
	vp9_pop_neon: @ PROC
    vld1.i64            {d8, d9, d10, d11}, [r0]!
    vld1.i64            {d12, d13, d14, d15}, [r0]!
    bx              lr

	.size vp9_pop_neon, .-vp9_pop_neon    @ ENDP


	.section	.note.GNU-stack,"",%progbits
