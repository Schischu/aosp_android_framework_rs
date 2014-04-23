/*
 * Copyright (C) 2014 The Android Open Source Project
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

/*
        r0 = dst
        r1 = y0 base pointer
        r2 = y1 base pointer
        r3 = y2 base pointer
        sp(r4) = y3 base pointer
        sp(r5) = (int) x1
	sp(q15)= (int) width - 1
        sp(r6) = (int) count
        sp(s0) = (float) scale
        sp(s1) = (float) yf
*/

#define ENTRY(f) .text; .align 0; .globl f; .type f,#function; f: .fnstart
#define END(f) .fnend; .size f, .-f;

# static float4 cubicInterpolate(float4 p0,float4 p1,float4 p2,float4 p3, float x) {
#    return  p1 + 
#            0.5f * x * 
#            (p2 - p0 + x * 
#            (2.f * p0 - 5.f * p1 + 4.f * p2 - p3 + x * 
#            (3.f * (p1 - p2) + p3 - p0)));
#}


# r6      pixel x offset


# s0, 1, 2, 3   d0, 1   q0      scale, yf, xf, (int)floor(xf)
# s4, 5, 6, 7   d2, 3   q1      (float)floor(xf),  *,  *,  0.5f
# s8, 9, 10,11  d4, 5   q2      2.0f, 3.0f, 4.0f, 5.0f
# s12,13,14,15  d6, 7   q3      idx
# s16,17,18,19  d8, 9   q4      0, 0, 0, 0
# s20,21,22,23  d10,11  q5      y1
# s24,25,26,27  d12,13  q6      y2
# s28,29,30,31  d14,15  q7      y3
#               d16,17  q8
#               d18,19  q9      
#               d20,21  q10     p0
#               d22,23  q11     p1
#               d24,25  q12     p2
#               d26,27  q13     p3
#               d28,29  q14     -1, 0, 1, 2
#               d30,31  q15     (w-1), (w-1), (w-1), (w-1)


floats:
.single 0.5 
.single 2.0
.single 3.0
.single 4.0
.single 5.0

ENTRY(rsdIntrinsicResizeB4_K)
        push            {r4-r8, r10, r11, lr}
        vpush           {q4-q7}

        ldr r4, [sp, #32+64]        /* yp3 */
        ldr r5, [sp, #32+64 + 4]        /* x1 */
        ldr r8, [sp, #32+64 + 8]   /* count */
        ldr r6, [sp, #32+64 + 12]   /* count */
        vldr s0, [sp, #32+64 +16]    /* scale */
        vldr s1, [sp, #32+64 +20]    /* yf */

	vdup.32 q15, r8

	mov r8, #-2
	vmov.32 s16, r8
	mov r8, #-1
	vmov.32 s17, r8
	mov r8, #0
	vmov.32 s18, r8
	mov r8, #1
	vmov.32 s19, r8
	vmov q14, q4
	veor q4, q4

        ldr r8, =0x3f000000
        vmov.32 d3[1], r8
        ldr r8, =0x40000000
        vmov.32 d4[0], r8
        ldr r8, =0x40400000
        vmov.32 d4[1], r8
        ldr r8, =0x40800000
        vmov.32 d5[0], r8
        ldr r8, =0x40a00000
        vmov.32 d5[1], r8


1:
        #float xf = x1 * cp->scaleX;
        vmov.32 s2, r5
        vcvt.f32.u32 s2, s2
        vmul.f32 s2, s2, s0
        
        # (int)s3 = floor(xf)   // safe because xf is > 0 
        vcvt.u32.f32 s3, s2

        # (float)s4 = floor(xf)
        vcvt.f32.u32 s4, s3

        # int startx = (int) floor(xf - 2);
	vdup.32 q3, d1[1]
	vadd.s32 q3, q3, q14
	vmax.s32 q3, q3, q4
	vmin.s32 q3, q3, q15

	vmov.32 r7, d6[0]
        add r8, r1, r7, lsl #2
        vld1.32 d6[0], [r8]
        add r8, r2, r7, lsl #2
        vld1.32 d10[0], [r8]
        add r8, r3, r7, lsl #2
        vld1.32 d12[0], [r8]
        add r8, r4, r7, lsl #2
        vld1.32 d14[0], [r8]

	vmov.32 r7, d6[1]
        add r8, r1, r7, lsl #2
        vld1.32 d6[1], [r8]
        add r8, r2, r7, lsl #2
        vld1.32 d10[1], [r8]
        add r8, r3, r7, lsl #2
        vld1.32 d12[1], [r8]
        add r8, r4, r7, lsl #2
        vld1.32 d14[1], [r8]

	vmov.32 r7, d7[0]
        add r8, r1, r7, lsl #2
        vld1.32 d7[0], [r8]
        add r8, r2, r7, lsl #2
        vld1.32 d11[0], [r8]
        add r8, r3, r7, lsl #2
        vld1.32 d13[0], [r8]
        add r8, r4, r7, lsl #2
        vld1.32 d15[0], [r8]

	vmov.32 r7, d7[1]
        add r8, r1, r7, lsl #2
        vld1.32 d7[1], [r8]
        add r8, r2, r7, lsl #2
        vld1.32 d11[1], [r8]
        add r8, r3, r7, lsl #2
        vld1.32 d13[1], [r8]
        add r8, r4, r7, lsl #2
        vld1.32 d15[1], [r8]

        #vmov.32 r7, s3
        #sub r7, r7, #2

#vdup.32 q3, d6[0]


        # xf = xf - floor(xf);
        vsub.f32 s2, s2, s4
        
#mov r8, #0
#ldr r8, [r8]


        vmovl.u8 q10, d7
        vmovl.u8 q9, d6
        vmovl.u16 q13, d21
        vmovl.u16 q12, d20
        vmovl.u16 q11, d19
        vmovl.u16 q10, d18
        vcvt.f32.u32 q13, q13
        vcvt.f32.u32 q12, q12
        vcvt.f32.u32 q11, q11
        vcvt.f32.u32 q10, q10

        #  x * (3.f * (p1 - p2) + p3 - p0)
        vsub.f32 q9, q11, q12
        vsub.f32 q8, q13, q10
        vmla.f32 q8, q9, d4[1]
        vmul.f32 q8, q8, d1[0]
        #      ((2.f * p0) - (5.f * p1) + (4.f * p2) - p3 + (q8)
        vmul.f32 q9, q10, d4[0]
        vmls.f32 q9, q11, d5[1]
        vmla.f32 q9, q12, d5[0]
        vsub.f32 q9, q9, q13
        vadd.f32 q9, q9, q8
        # p1 +  0.5f * x * ((p2 - p0) + x * (q9)
        vsub.f32 q8, q12, q10
        vmla.f32 q8, q9, d1[0]
        vmul.f32 q8, q8, d1[0]
        vmul.f32 q8, q8, d3[1]
        vadd.f32 q3, q8, q11


        vmovl.u8 q10, d11
        vmovl.u8 q9, d10
        vmovl.u16 q13, d21
        vmovl.u16 q12, d20
        vmovl.u16 q11, d19
        vmovl.u16 q10, d18
        vcvt.f32.u32 q13, q13
        vcvt.f32.u32 q12, q12
        vcvt.f32.u32 q11, q11
        vcvt.f32.u32 q10, q10

        #  x * (3.f * (p1 - p2) + p3 - p0)
        vsub.f32 q9, q11, q12
        vsub.f32 q8, q13, q10
        vmla.f32 q8, q9, d4[1]
        vmul.f32 q8, q8, d1[0]
        #      ((2.f * p0) - (5.f * p1) + (4.f * p2) - p3 + (q8)
        vmul.f32 q9, q10, d4[0]
        vmls.f32 q9, q11, d5[1]
        vmla.f32 q9, q12, d5[0]
        vsub.f32 q9, q9, q13
        vadd.f32 q9, q9, q8
        # p1 +  0.5f * x * ((p2 - p0) + x * (q9)
        vsub.f32 q8, q12, q10
        vmla.f32 q8, q9, d1[0]
        vmul.f32 q8, q8, d1[0]
        vmul.f32 q8, q8, d3[1]
        vadd.f32 q5, q8, q11


        vmovl.u8 q10, d13
        vmovl.u8 q9, d12
        vmovl.u16 q13, d21
        vmovl.u16 q12, d20
        vmovl.u16 q11, d19
        vmovl.u16 q10, d18
        vcvt.f32.u32 q13, q13
        vcvt.f32.u32 q12, q12
        vcvt.f32.u32 q11, q11
        vcvt.f32.u32 q10, q10

        #  x * (3.f * (p1 - p2) + p3 - p0)
        vsub.f32 q9, q11, q12
        vsub.f32 q8, q13, q10
        vmla.f32 q8, q9, d4[1]
        vmul.f32 q8, q8, d1[0]
        #      ((2.f * p0) - (5.f * p1) + (4.f * p2) - p3 + (q8)
        vmul.f32 q9, q10, d4[0]
        vmls.f32 q9, q11, d5[1]
        vmla.f32 q9, q12, d5[0]
        vsub.f32 q9, q9, q13
        vadd.f32 q9, q9, q8
        # p1 +  0.5f * x * ((p2 - p0) + x * (q9)
        vsub.f32 q8, q12, q10
        vmla.f32 q8, q9, d1[0]
        vmul.f32 q8, q8, d1[0]
        vmul.f32 q8, q8, d3[1]
        vadd.f32 q6, q8, q11


        vmovl.u8 q10, d15
        vmovl.u8 q9, d14
        vmovl.u16 q13, d21
        vmovl.u16 q12, d20
        vmovl.u16 q11, d19
        vmovl.u16 q10, d18
        vcvt.f32.u32 q13, q13
        vcvt.f32.u32 q12, q12
        vcvt.f32.u32 q11, q11
        vcvt.f32.u32 q10, q10

        #  x * (3.f * (p1 - p2) + p3 - p0)
        vsub.f32 q9, q11, q12
        vsub.f32 q8, q13, q10
        vmla.f32 q8, q9, d4[1]
        vmul.f32 q8, q8, d1[0]
        #      ((2.f * p0) - (5.f * p1) + (4.f * p2) - p3 + (q8)
        vmul.f32 q9, q10, d4[0]
        vmls.f32 q9, q11, d5[1]
        vmla.f32 q9, q12, d5[0]
        vsub.f32 q9, q9, q13
        vadd.f32 q9, q9, q8
        # p1 +  0.5f * x * ((p2 - p0) + x * (q9)
        vsub.f32 q8, q12, q10
        vmla.f32 q8, q9, d1[0]
        vmul.f32 q8, q8, d1[0]
        vmul.f32 q8, q8, d3[1]
        vadd.f32 q7, q8, q11



        #  x * (3.f * (p1 - p2) + p3 - p0)
        vsub.f32 q9, q5, q6
        vsub.f32 q8, q7, q3
        vmla.f32 q8, q9, d4[1]
        vmul.f32 q8, q8, d0[1]
        #      ((2.f * p0) - (5.f * p1) + (4.f * p2) - p3 + (q8)
        vmul.f32 q9, q3, d4[0]
        vmls.f32 q9, q5, d5[1]
        vmla.f32 q9, q6, d5[0]
        vsub.f32 q9, q9, q7
        vadd.f32 q9, q9, q8
        # p1 +  0.5f * x * ((p2 - p0) + x * (q9)
        vsub.f32 q8, q6, q3
        vmla.f32 q8, q9, d0[1]
        vmul.f32 q8, q8, d0[1]
        vmul.f32 q8, q8, d3[1]
        vadd.f32 q8, q8, q5


        vcvt.u32.f32 q8, q8
        vqmovn.s32 d16, q8
        vqmovun.s16 d16, q8

        vst1.32 d16[0], [r0]! 

        /* Are we done yet? */
	add r5, r5, #1
        subs r6, r6, #1
        bne 1b

        /* We're done, bye! */
        vpop            {q4-q7}
        pop             {r4-r8, r10, r11, lr}
        bx              lr
END(rsdIntrinsicResizeB4_K)

