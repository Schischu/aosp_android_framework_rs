/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma version(1)
#pragma rs java_package_name(com.android.rs.reducetest)

// #include "shared.rsh"
// These constants are all we need from CTS's shared.rsh.
static const int RS_MSG_TEST_PASSED = 100;
static const int RS_MSG_TEST_FAILED = 101;

static bool failed = false;

// Builds a periodic sequence of values whose partial sums shouldn't
// overflow any primitive type.
//
// One period of the sequence is
// -15, -14, ..., -1, 0, 1, ..., 14, 15, 15, 14, ... 1, 0, -1, ..., -14, -15
static int seq(uint32_t pos) {
  return (((int) pos % 31) - 15) * (pos % 62 < 31 ? 1 : -1);
}

// Returns sum(seq(0), ..., seq(pos - 1)).
static int partial_sum_seq(uint32_t pos) {
  // Number of values to sum
  int n = pos % 31;
  // Partial sum of n values of the sequence -15, -14, -13, ...
  int psum = (n == 0) ? 0 : (n - 31) * n / 2;
  // The partial sums have a full period of 62. The second half of the
  // period is a repetition of the first half of the period with the
  // opposite sign.
  return (pos % 62 < 31 ? psum : -psum);
}

// Functions to compute the partial sum of a subrange

#define MAKE_PARTIAL_SUM(TYPE)                               \
static TYPE partial_sum_##TYPE(uint32_t x1, uint32_t x2) {   \
  return (TYPE) (partial_sum_seq(x2) - partial_sum_seq(x1)); \
}

// Functions to compare values

#define MAKE_COMPARISON_SCALAR(TYPE)             \
static bool equals_##TYPE(TYPE lhs, TYPE rhs) {  \
  return lhs == rhs;                             \
}

#define MAKE_COMPARISON_VEC2(TYPE)               \
static bool equals_##TYPE(TYPE lhs, TYPE rhs) {  \
  return lhs[0] == rhs[0] && lhs[1] == rhs[1];   \
}

#define MAKE_COMPARISON_VEC3(TYPE)               \
static bool equals_##TYPE(TYPE lhs, TYPE rhs) {  \
  return lhs[0] == rhs[0] && lhs[1] == rhs[1] && \
         lhs[2] == rhs[2];                       \
}

#define MAKE_COMPARISON_VEC4(TYPE)               \
static bool equals_##TYPE(TYPE lhs, TYPE rhs) {  \
  return lhs[0] == rhs[0] && lhs[1] == rhs[1] && \
         lhs[2] == rhs[2] && lhs[3] == rhs[3];   \
}

// Functions to initialize values

#define MAKE_INITIALIZER(TYPE)                 \
static TYPE initializer_##TYPE(uint32_t pos) { \
  return (TYPE) seq(pos);                      \
}

// Invokable that sets up an rs_allocation

#define MAKE_SET_UP_FUNCTION(TYPE)                            \
void set_up_input_##TYPE(rs_allocation alloc) {               \
  for (uint32_t i = 0; i < rsAllocationGetDimX(alloc); ++i) { \
    rsSetElementAt_##TYPE(alloc, initializer_##TYPE(i), i);   \
  }                                                           \
}

// Reduce kernel

#define MAKE_REDUCE_KERNEL(TYPE)       \
TYPE __attribute__((kernel("reduce"))) \
add_##TYPE(TYPE lhs, TYPE rhs) {       \
  return lhs + rhs;                    \
}

// Invokable that checks the result of a reduction

#define MAKE_VERIFY_FUNCTION(TYPE)                                             \
void verify_output_##TYPE(rs_allocation alloc, uint32_t start, uint32_t end) { \
  TYPE elem = rsGetElementAt_##TYPE(alloc, 0);                                 \
  TYPE correct = partial_sum_##TYPE(start, end);                               \
  if (!equals_##TYPE(elem, correct)) {                                         \
    rsDebug("incorrect result", elem);                                         \
    rsDebug("expected result", correct);                                       \
    failed = true;                                                             \
  }                                                                            \
}

#define XSCALAR(TYPE)          \
  MAKE_PARTIAL_SUM(TYPE)       \
  MAKE_COMPARISON_SCALAR(TYPE) \
  MAKE_INITIALIZER(TYPE)       \
  MAKE_SET_UP_FUNCTION(TYPE)   \
  MAKE_REDUCE_KERNEL(TYPE)     \
  MAKE_VERIFY_FUNCTION(TYPE)

#define XVEC2(TYPE)            \
  MAKE_PARTIAL_SUM(TYPE)       \
  MAKE_COMPARISON_VEC2(TYPE)   \
  MAKE_INITIALIZER(TYPE)       \
  MAKE_SET_UP_FUNCTION(TYPE)   \
  MAKE_REDUCE_KERNEL(TYPE)     \
  MAKE_VERIFY_FUNCTION(TYPE)

#define XVEC3(TYPE)            \
  MAKE_PARTIAL_SUM(TYPE)       \
  MAKE_COMPARISON_VEC3(TYPE)   \
  MAKE_INITIALIZER(TYPE)       \
  MAKE_SET_UP_FUNCTION(TYPE)   \
  MAKE_REDUCE_KERNEL(TYPE)     \
  MAKE_VERIFY_FUNCTION(TYPE)

#define XVEC4(TYPE)            \
  MAKE_PARTIAL_SUM(TYPE)       \
  MAKE_COMPARISON_VEC4(TYPE)   \
  MAKE_INITIALIZER(TYPE)       \
  MAKE_SET_UP_FUNCTION(TYPE)   \
  MAKE_REDUCE_KERNEL(TYPE)     \
  MAKE_VERIFY_FUNCTION(TYPE)

#include "reducePrimitiveTypesTable.rsh"

REDUCE_PRIMITIVE_TYPES_TABLE

#undef REDUCE_PRIMITIVE_TYPES_TABLE

// Special case for half (no rsDebug() available)

#undef MAKE_VERIFY_FUNCTION

#define MAKE_VERIFY_FUNCTION(TYPE)                                             \
void verify_output_##TYPE(rs_allocation alloc, uint32_t start, uint32_t end) { \
  TYPE elem = * (TYPE *) rsGetElementAt(alloc, 0);                             \
  TYPE correct = partial_sum_##TYPE(start, end);                               \
  if (!equals_##TYPE(elem, correct)) {                                         \
    failed = true;                                                             \
  }                                                                            \
}

XSCALAR(half)
XVEC2(half2)
XVEC3(half3)
XVEC4(half4)

#undef XVEC4
#undef XVEC3
#undef XVEC2
#undef XSCALAR

// Special case for bool (since bool itself is a macro, it can't be
// handled by the above code).

void set_up_input_bool(rs_allocation alloc) {
  for (uint32_t i = 0; i < rsAllocationGetDimX(alloc); ++i) {
    bool val = i % 2;
    rsSetElementAt(alloc, &val, i);
  }
}

bool __attribute__((kernel("reduce")))
add_bool(bool lhs, bool rhs) {
  return lhs | rhs;
}

void verify_output_bool(rs_allocation alloc, uint32_t start, uint32_t end) {
  bool val = * (bool *) rsGetElementAt(alloc, 0);
  bool correct = (start % 2 == 1) || (end - start > 1);
  if (val != correct) {
    rsDebug("incorrect result", val);
    rsDebug("expected result", correct);
    failed = true;
  }
}

void checkError() {
  if (failed) {
    rsSendToClientBlocking(RS_MSG_TEST_FAILED);
  } else {
    rsSendToClientBlocking(RS_MSG_TEST_PASSED);
  }
}
