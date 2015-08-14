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

// merge_interval is an example of an associative non-commutative
// binary function.
//
// The input foo represents an interval (foo[0], foo[1]), assuming
// foo[0] <= foo[1]. The operation merges intervals (a, b), (c, d)
// into (a, d) if b == c, otherwise it returns (-1, -1).
int2 __attribute__((kernel("reduce"))) merge_intervals(int2 lhs, int2 rhs) {
  const int2 bad = {-1, -1};

  if (lhs[0] > lhs[1]) {
    return bad;
  }

  if (rhs[0] > rhs[1]) {
    return bad;
  }

  if (lhs[1] != rhs[0]) {
    return bad;
  }

  const int2 out = {lhs[0], rhs[1]};

  return out;
}
