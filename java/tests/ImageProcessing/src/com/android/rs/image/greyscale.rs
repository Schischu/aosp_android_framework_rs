/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include "ip.rsh"
#pragma rs_fp_relaxed

const static float3 gMonoMult = {0.299f, 0.587f, 0.114f};

const static half gMonoMultR = 0.299f;
const static half gMonoMultG = 0.587f;
const static half gMonoMultB = 0.114f;

uchar4 __attribute__((kernel)) root(uchar4 v_in) {
    half r = v_in.r;
    half g = v_in.g;
    half b = v_in.b;
    r *= gMonoMultR;
    g *= gMonoMultG;
    b *= gMonoMultB;
    uchar4 ret = {r, g, b, 255};
    return ret;
}

uchar RS_KERNEL toU8(uchar4 v_in) {
    float4 f4 = convert_float4(v_in);
    return (uchar)dot(f4.rgb, gMonoMult);
}

uchar4 RS_KERNEL toU8_4(uchar v_in) {
    return (uchar4)v_in;
}

