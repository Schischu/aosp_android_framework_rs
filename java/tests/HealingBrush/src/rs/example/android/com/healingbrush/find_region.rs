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
#pragma rs java_package_name(com.example.android.rs.sample)
#pragma rs_fp_relaxed

rs_allocation image;
rs_allocation border_values; // float3
rs_allocation border_coords; //int2
int borderLength;

float3 __attribute__((kernel))extractBorder(uint2 in) {
  return convert_float3(rsGetElementAt_uchar4(image, in.x, in.y).xyz);
}

int imagePosX;
int imagePosY;

float __attribute__((kernel)) bordercorrelation(uint32_t x, uint32_t y) {
  float sum = 0;
  int dx = x-imagePosX;
  int dy = y-imagePosY;
  for(int i = 0 ; i < borderLength; i++) {
    int2  coord = rsGetElementAt_int2(border_coords, i);
    float3 orig = convert_float3(rsGetElementAt_uchar4(image, coord.x + dx, coord.y + dy).xyz);
    float3 candidate = rsGetElementAt_float3(border_values, i).xyz;
    sum += distance(orig, candidate);
  }
  return sum;
}

