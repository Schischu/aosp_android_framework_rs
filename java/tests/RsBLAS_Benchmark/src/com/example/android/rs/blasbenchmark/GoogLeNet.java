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

package com.example.android.rs.blasbenchmark;

import android.support.v8.renderscript.*;
import android.util.Log;
import java.util.ArrayList;
import java.util.Random;
import java.lang.Math;

public class GoogLeNet extends TestBase {
    ScriptIntrinsicBLAS mBLAS;
    private ArrayList<Allocation> matA;
    private ArrayList<Allocation> matB;
    private ArrayList<Allocation> matC;

    private int usage;
    private byte[] byteOut;
    private float[] floatOut;
    
    // private int googlenet_gemm_sizes[] = {12544, 64, 147, 3136, 64, 64, 3136, 192, 576, 784, 64, 192};

    private int googlenet_gemm_sizes[] = {
        12544, 64, 147,
        3136, 64, 64,
        3136, 192, 576,
        784, 64, 192,
        784, 96, 192,
        784, 128, 864,
        784, 16, 192,
        784, 32, 400,
        784, 32, 192,
        784, 128, 256,
        784, 128, 256,
        784, 192, 1152,
        784, 32, 256,
        784, 96, 800,
        784, 64, 256,
        196, 192, 480,
        196, 96, 480,
        196, 204, 864,
        196, 16, 480,
        196, 48, 400,
        196, 64, 480,
        196, 160, 508,
        196, 112, 508,
        196, 224, 1008,
        196, 24, 508,
        196, 64, 600,
        196, 64, 508,
        196, 128, 512,
        196, 128, 512,
        196, 256, 1152,
        196, 24, 512,
        196, 64, 600,
        196, 64, 512,
        196, 112, 512,
        196, 144, 512,
        196, 288, 1296,
        196, 32, 512,
        196, 64, 800,
        196, 64, 512,
        196, 256, 528,
        196, 160, 528,
        196, 320, 1440,
        196, 32, 528,
        196, 128, 800,
        196, 128, 528,
        49, 256, 832,
        49, 160, 832,
        49, 320, 1440,
        49, 48, 832,
        49, 128, 1200,
        49, 128, 832,
        49, 384, 832,
        49, 192, 832,
        49, 384, 1728,
        49, 48, 832,
        49, 128, 1200,
        49, 128, 832,
        16, 128, 508,
        1, 1024, 2048,
        1, 1008, 1024,
        16, 128, 528,
        1, 1024, 2048,
        1, 1008, 1024,
        1, 1008, 1024,
    };

    GoogLeNet(int usage) {
        this.usage = usage;
    }

    public void createTest() {
        mBLAS = ScriptIntrinsicBLAS.create(mRS);
        byteOut = new byte[2];
        floatOut = new float[2];
        setTest();
    }

    private int roundUp(int input, int roundN) {
        int result = input;
        if (input % roundN > 0) {
            result += roundN - input % roundN;
        }
        return result;
    }


    private byte[] getByteArr(int len) {
        byte[] result = new byte[len];
        for (int i=0; i<len; i++) {
            result[i] = 2;
        }
        return result;
    }

    private float[] getFloatArr(int len) {
        float[] result = new float[len];
        for (int i=0; i<len; i++) {
            result[i] = 1.2f;
        }
        return result;
    }

    private void setTest() {
        matA = new ArrayList<Allocation>();
        matB = new ArrayList<Allocation>();
        matC = new ArrayList<Allocation>();

        Element e;

        for (int i=0; i<googlenet_gemm_sizes.length; i+=3) {
            if (usage == 1) {
                int m = googlenet_gemm_sizes[i];
                int n = googlenet_gemm_sizes[i+1];
                int k = googlenet_gemm_sizes[i+2];
                e = Element.F32(mRS);
                Type.Builder builder = new Type.Builder(mRS, e);
                Type a_type = builder.setX(k).setY(m).create();
                Type b_type = builder.setX(n).setY(k).create();
                Type c_type = builder.setX(n).setY(m).create();

                Allocation A = Allocation.createTyped(mRS, a_type);
                Allocation B = Allocation.createTyped(mRS, b_type);
                Allocation C = Allocation.createTyped(mRS, c_type);

                A.copyFrom(getFloatArr(k*m));
                B.copyFrom(getFloatArr(k*n));
                C.copyFrom(getFloatArr(n*m));

                matA.add(A);
                matB.add(B);
                matC.add(C);
            } else if (usage == 2) {
                int m = googlenet_gemm_sizes[i];
                int n = googlenet_gemm_sizes[i+1];
                int k = googlenet_gemm_sizes[i+2];
                e = Element.U8(mRS);
                Type.Builder builder = new Type.Builder(mRS, e);
                Type a_type = builder.setX(k).setY(m).create();
                Type b_type = builder.setX(k).setY(n).create();
                Type c_type = builder.setX(n).setY(m).create();

                Allocation A = Allocation.createTyped(mRS, a_type);
                Allocation B = Allocation.createTyped(mRS, b_type);
                Allocation C = Allocation.createTyped(mRS, c_type);


                A.copyFrom(getByteArr(k*m));
                B.copyFrom(getByteArr(k*n));
                C.copyFrom(getByteArr(n*m));

                matA.add(A);
                matB.add(B);
                matC.add(C);
            } else if (usage == 3){
                int m = roundUp(googlenet_gemm_sizes[i], 8);
                int n = roundUp(googlenet_gemm_sizes[i+1], 4);
                int k = roundUp(googlenet_gemm_sizes[i+2], 4);
                e = Element.F32(mRS);
                Type.Builder builder = new Type.Builder(mRS, e);
                Type a_type = builder.setX(k).setY(m).create();
                Type b_type = builder.setX(n).setY(k).create();
                Type c_type = builder.setX(n).setY(m).create();

                Allocation A = Allocation.createTyped(mRS, a_type);
                Allocation B = Allocation.createTyped(mRS, b_type);
                Allocation C = Allocation.createTyped(mRS, c_type);

                A.copyFrom(getFloatArr(k*m));
                B.copyFrom(getFloatArr(k*n));
                C.copyFrom(getFloatArr(n*m));

                matA.add(A);
                matB.add(B);
                matC.add(C);
            } else {
                int m = roundUp(googlenet_gemm_sizes[i], 8);
                int n = roundUp(googlenet_gemm_sizes[i+1], 4);
                int k = roundUp(googlenet_gemm_sizes[i+2], 4);
                e = Element.U8(mRS);
                Type.Builder builder = new Type.Builder(mRS, e);
                Type a_type = builder.setX(k).setY(m).create();
                Type b_type = builder.setX(k).setY(n).create();
                Type c_type = builder.setX(n).setY(m).create();

                Allocation A = Allocation.createTyped(mRS, a_type);
                Allocation B = Allocation.createTyped(mRS, b_type);
                Allocation C = Allocation.createTyped(mRS, c_type);


                A.copyFrom(getByteArr(k*m));
                B.copyFrom(getByteArr(k*n));
                C.copyFrom(getByteArr(n*m));

                matA.add(A);
                matB.add(B);
                matC.add(C);
            }
        }
    }

    public void runTest() {
        if (usage == 1 || usage == 3) {
            for (int i=0; i<googlenet_gemm_sizes.length/3; i++) {
                mBLAS.SGEMM(ScriptIntrinsicBLAS.NO_TRANSPOSE, ScriptIntrinsicBLAS.NO_TRANSPOSE,
                            1.0f, matA.get(i), matB.get(i), 0.f, matC.get(i));
                matC.get(i).copy1DRangeTo(0, 1, floatOut);
            }
        } else {
            int a_offset = 1;
            int b_offset = 1;
            int c_mult_int = 1;
            int c_offset = 1;
            for (int i=0; i<googlenet_gemm_sizes.length/3; i++) {
                mBLAS.BNNM(matA.get(i), a_offset, matB.get(i), b_offset, matC.get(i), c_offset, c_mult_int);
                matC.get(i).copy1DRangeTo(0, 1, byteOut);
            }
        }
    }

    public String getTestInfo() {
        return "GoogLeNetTest: " + googlenet_gemm_sizes.length / 3;
    }
}

