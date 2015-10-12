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

# TODO Once Yang's patches to API generator land:
#     1. Fix version number appropriately
#     2. Mark rsCreate{Primitive,Vector}Allocation as intrinsic
#     3. Mark rsCreateAllocationTyped as internal

header:
summary: Alloccation Creation Functions
description:
 The functions below can be used to create allocations from an invokable.
end:

function: rsCreatePrimitiveAllocation
version: 24
ret: rs_allocation
arg: rs_data_type data_type
arg: uint32_t x
summary: Create an allocation of given type
description:
 Creates an allocation of type rs_data_type.

 Use the x, y, and z variant to create a three-dimensional allocation.
 Likewise, use the x, y variant for a two-dimensional allocation and the
 x-only variant for a single-dimensional allocaion.

test: none
end:

function: rsCreatePrimitiveAllocation
version: 24
ret: rs_allocation
arg: rs_data_type data_type
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsCreatePrimitiveAllocation
version: 24
ret: rs_allocation
arg: rs_data_type data_type
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
test: none
end:

function: rsCreateVectorAllocation
version: 24
ret: rs_allocation
arg: rs_data_type data_type
arg: size_t vecSize
arg: uint32_t x
summary: Create a vector allocation of given type
description:
 Creates a vector allocation with vecSize elements of type rs_data_type

 Use the x, y, and z variant to create a three-dimensional allocation.
 Likewise, use the x, y variant for a two-dimensional allocation and the
 x-only variant for a single-dimensional allocaion.

test: none
end:

function: rsCreateVectorAllocation
version: 24
ret: rs_allocation
arg: rs_data_type data_type
arg: size_t vecSize
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsCreateVectorAllocation
version: 24
ret: rs_allocation
arg: rs_data_type data_type
arg: size_t vecSize
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
test: none
end:

function: rsCreateAllocationTyped
version: 24
ret: rs_allocation
arg: rs_data_type data_type
arg: rs_data_kind data_kind
arg: uint32_t vecSize
arg: uint32_t dimX
arg: uint32_t dimY
arg: uint32_t dimz
summary: (Internal API) Create an Allocation Launch a kernel in the current Script (with the slot number)
description:
test: none
end:

