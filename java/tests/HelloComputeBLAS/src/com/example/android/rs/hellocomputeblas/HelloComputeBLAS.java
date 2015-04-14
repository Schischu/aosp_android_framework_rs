/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.example.android.rs.hellocomputeblas;

import android.app.Activity;
import android.os.Bundle;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap;
import android.renderscript.*;
import android.widget.ImageView;

import java.util.Random;

public class HelloComputeBLAS extends Activity {
    private Bitmap mBitmapIn;
    private Bitmap mBitmapOut;

    private RenderScript mRS;
    private Allocation mInAllocation;
    private Allocation mOutAllocation;
    private ScriptC_mono mScript;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        mBitmapIn = loadBitmap(R.drawable.data);
        mBitmapOut = Bitmap.createBitmap(mBitmapIn.getWidth(), mBitmapIn.getHeight(),
                                         mBitmapIn.getConfig());

        ImageView in = (ImageView) findViewById(R.id.displayin);
        in.setImageBitmap(mBitmapIn);

        ImageView out = (ImageView) findViewById(R.id.displayout);
        out.setImageBitmap(mBitmapOut);

        createScript();
        testSmallMatrices();
        testMediumMatrices1();
        testMediumMatrices2();
        testRealData();
    }

    static {
        System.loadLibrary("8bgemm");
    }

    native void getData(byte[] a, byte[] b, byte[] c);

    // Adds pseudo-random errors to the byte array with the specified frequency.
    // For example if the frequency is 0.1, then 10% of the values will be altered.
    // The max_delta controls how large the biggest changes are. The exact
    // frequency of changes is not guaranteed, since it's implemented as a
    // probability of changing any individual value.
    private void addByteNoise(byte[] data, int count, float frequency, int maxDelta) {
        Random rand = new Random();
        for (int n = 0; n < count; ++n) {
            if (rand.nextFloat() < frequency) {
                final int originalValue = data[n];
                final float direction = rand.nextFloat();
                int delta = (int)(Math.ceil(rand.nextFloat() * maxDelta));
                if (direction < 0.5f) {
                    delta = -delta;
                }
                int newValue = (originalValue + delta);
                if (newValue < -127) {
                    newValue = -127;
                }
                if (newValue > 127) {
                    newValue = 127;
                }
                data[n] = (byte)(newValue);
            }
        }
    }

    // In Java, the eight-bit 'byte' type is signed, but the API for the 8-bit
    // matrix multiplication deals with unsigned bytes. This is a convenience
    // function that converts arrays of unsigned ints to their equivalent
    // representations as signed bytes. For example, the bit pattern 0xff is 255
    // as an unsigned value, but -127 as a Java signed byte. So if you pass in an
    // array of int[] {255} into this function, you'll get back byte[] {-127}.
    private byte[] unsignedToSignedByte(int[] input) {
        byte[] output = new byte[input.length];
        for (int i = 0; i < input.length; ++i) {
            output[i] = (byte)(input[i]);
        }
        return output;
    }

    private void createScript() {
        mRS = RenderScript.create(this);

        mInAllocation = Allocation.createFromBitmap(mRS, mBitmapIn,
                                                    Allocation.MipmapControl.MIPMAP_NONE,
                                                    Allocation.USAGE_SCRIPT);
        mOutAllocation = Allocation.createFromBitmap(mRS, mBitmapOut,
                                                     Allocation.MipmapControl.MIPMAP_NONE,
                                                     Allocation.USAGE_SCRIPT);

        mScript = new ScriptC_mono(mRS);

        mScript.forEach_root(mInAllocation, mOutAllocation);
        mOutAllocation.copyTo(mBitmapOut);
    }

    // This test multiplies a couple of small 8-bit matrices, and compares the
    // results with hand-calculated expectations.
    private void testSmallMatrices() {
        // The A matrix is:
        // |   1 |   4 |
        // |   2 |   5 |
        // |   3 |   6 |
        byte[] a_data = unsignedToSignedByte(new int[] {
                1, 2, 3,
                4, 5, 6,
        });
        final int a_rows = 3;
        final int a_cols = 2;
        final int a_offset = 0;
        // The B matrix is:
        // |  -1 |  -2 |  -3 |  -4 |
        // |  -5 |  -6 |  -7 |  -8 |
        // |  -9 | -10 | -11 | -12 |
        byte[] b_data = unsignedToSignedByte(new int[] {
                11, 7, 3,
                10, 6, 2,
                9, 5, 1,
                8, 4, 0,
        });
        final int b_cols = 4;
        final int b_offset = 12;
        // EightBitGemm implements C = B.transposed() * A,
        // so we expect to get these results:
        // 1*-1 + 2*-5 + 3*-9 + 128 = 90
        // 1*-2 + 2*-6 + 3*-10 + 128 = 84
        // 1*-3 + 2*-7 + 3*-11 + 128 = 78
        // 1*-4 + 2*-8 + 3*-12 + 128 = 72
        // 4*-1 + 5*-5 + 6*-9 + 128 = 45
        // 4*-2 + 5*-6 + 6*-10 + 128 = 30
        // 4*-3 + 5*-7 + 6*-11 + 128 = 15
        // 4*-4 + 5*-8 + 6*-12 + 128 = 0
        // | 90 |  45 |
        // | 84 |  30 |
        // | 78 | 15 |
        // | 72 | 0 |
        final int c_offset = 128;
        final int c_shift = 21;
        final int c_mult_int = (1 << c_shift);
        byte[] expected_data = unsignedToSignedByte(new int[] {
              90, 84, 78, 72,
              45, 30, 15, 0,
        });

        final int m = a_cols;
        final int n = b_cols;
        final int k = a_rows;

        byte[] c_byte_output = runGemm(m, n, k, a_data, a_offset, b_data, b_offset,
                c_offset, c_mult_int);
        testWithTolerance(expected_data, c_byte_output, "small matrices");
    }

    // This test multiplies two medium-sized 8-bit matrices, and compares the
    // results with the expected values. The data itself is fairly arbitrary.
    private void testMediumMatrices1() {
        byte[] a_data = unsignedToSignedByte(new int[] {
                1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
                23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
                0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
                23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
        });
        final int a_rows = 11;
        final int a_cols = 5;
        final int a_offset = 0;
        byte[] b_data = unsignedToSignedByte(new int[] {
                0, 2, 4, 6, 8, 10, 1, 3, 5, 7, 9, 11,
                10, 12, 14, 16, 18, 20, 11, 13, 15, 17, 19, 21,
                20, 22, 24, 26, 28, 30, 21, 23, 25, 27, 29, 31,
                30, 32, 34, 36, 38, 40, 31, 33, 35, 37, 39, 41,
                40, 42, 44, 46, 48, 50, 41, 43, 45, 47, 49, 51,
                50, 52, 54, 56, 58, 60, 51, 53, 55, 57, 59, 61,
                60, 62, 64, 66, 68, 70, 61, 63, 65, 67, 69, 71,
        });
        final int b_cols = 7;
        final int b_offset = 10;
        final int c_offset = 16384;
        final int c_shift = 21;
        final int c_mult_int = (1 << (c_shift - 7));
        byte[] expected_data = unsignedToSignedByte(new int[] {
              126, 131, 135, 140, 146, 151, 155,
              121, 135, 148, 162, 176, 190, 202,
              116, 139, 161, 184, 206, 229, 249,
              128, 128, 129, 129, 129, 130, 130,
              118, 136, 155, 173, 191, 210, 226,
        });

        final int m = a_cols;
        final int n = b_cols;
        final int k = a_rows;

        byte[] c_byte_output = runGemm(m, n, k, a_data, a_offset, b_data, b_offset,
                c_offset, c_mult_int);
        testWithTolerance(expected_data, c_byte_output, "medium matrices #1");
    }

    // This test multiplies another two medium 8-bit matrices, and compares the
    // results with the expected values. The data here is arbitrary.
    private void testMediumMatrices2() {
        byte[] a_data = unsignedToSignedByte(new int[] {
                1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,
                1, 23, 2, 22, 3, 21, 4, 20, 5, 19, 6, 18, 7, 17, 8, 16, 9, 15, 10, 14, 11, 13, 12,
                23, 1, 22, 2, 21, 3, 20, 4, 19, 5, 18, 6, 17, 7, 16, 8, 15, 9, 14, 10, 13, 11, 12,
                1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
                3, 1, 4, 1, 5, 8, 2, 3, 1, 14, 11, 15, 18, 12, 13, 11, 14, 11, 15, 18, 12, 13, 11,
                8, 0, 5, 8, 1, 3, 7, 5, 7, 13, 10, 23, 13, 11, 17, 23, 12, 19, 17, 13, 14, 10, 19,
        });
        final int a_rows = 23;
        final int a_cols = 7;
        final int a_offset = 13;
        byte[] b_data = unsignedToSignedByte(new int[] {
                0, 2, 4, 6, 8, 10, 1, 3, 5, 7, 9, 11, 0, 2, 4, 6, 8, 10, 1, 3, 5, 7, 9,
                0, 20, 40, 60, 80, 10, 11, 13, 15, 17, 19, 21, 10, 12, 14, 6, 8, 10, 1, 3, 5, 7, 9,
                1, 21, 41, 61, 81, 11, 12, 14, 16, 18, 20, 22, 11, 13, 15, 7, 9, 11, 2, 4, 6, 8, 9,
                0, 19, 39, 59, 79, 9, 10, 12, 14, 16, 18, 20, 9, 11, 13, 5, 7, 9, 0, 2, 4, 6, 8,
                2, 22, 42, 62, 82, 12, 13, 15, 17, 19, 21, 23, 12, 14, 16, 8, 9, 12, 3, 5, 7, 9, 9,
                0, 18, 38, 58, 78, 8, 9, 11, 13, 15, 17, 19, 8, 10, 12, 4, 6, 8, 0, 1, 3, 5, 7,
                3, 23, 43, 63, 83, 13, 14, 16, 18, 20, 22, 24, 13, 15, 17, 9, 9, 13, 4, 6, 8, 9, 9,
                0, 17, 37, 57, 77, 7, 8, 10, 12, 14, 16, 18, 7, 9, 11, 3, 5, 7, 0, 0, 2, 4, 6,
                10, 20, 30, 40, 50, 1, 2, 3, 4, 5, 11, 12, 13, 14, 15, 21, 22, 23, 24, 25, 1, 2, 3,
        });
        final int b_cols = 9;
        final int b_offset = 23;
        final int c_offset = 2121;
        final int c_shift = 21;
        final int c_mult_int = 132359;
        byte[] expected_data = unsignedToSignedByte(new int[] {
              167, 53, 51, 54, 49, 55, 46,
              56, 116, 153, 232, 232, 234, 231,
              236, 232, 237, 174, 168, 131, 130,
              132, 129, 133, 128, 133, 134, 151,
              154, 152, 156, 151, 158, 150, 160,
              156, 255, 113, 106, 120, 98, 127,
              91, 134, 178, 231, 102, 97, 107,
              92, 111, 87, 116, 164, 187, 76,
              73, 78, 70, 81, 67, 83, 139,
        });

        final int m = a_cols;
        final int n = b_cols;
        final int k = a_rows;

        byte[] c_byte_output = runGemm(m, n, k, a_data, a_offset, b_data, b_offset,
                c_offset, c_mult_int);
        testWithTolerance(expected_data, c_byte_output, "medium matrices #2");
    }

    // This test takes a large set of real data captured from a convolutional
    // neural network solving a computer vision problem, and runs it through the
    // eight-bit matrix multiply. We test the results to make sure they're close
    // enough to be usable.
    private void testRealData() {
        int m = 256;
        int n = 192;
        int k = 1152;
        int a_offset = 0;
        int b_offset = 84;
        int c_mult_int = 3401;
        int c_offset = 74980;

        int a_count = (m * k);
        int b_count = (n * k);
        int c_count = (m * n);

        byte[] a_byte = new byte[a_count];
        byte[] b_byte = new byte[b_count];
        byte[] c_byte = new byte[c_count];

        getData(a_byte, b_byte, c_byte);
        byte[] c_byte_output = runGemm(m, n, k, a_byte, a_offset, b_byte, b_offset,
                c_offset, c_mult_int);
        testWithTolerance(c_byte, c_byte_output, "real data");
    }

    // Calls the Renderscript eight-bit GEMM intrinsic.
    private byte[] runGemm(int m, int n, int k, byte[] a_byte, int a_offset, byte[] b_byte,
        int b_offset, int c_offset, int c_mult_int) {

        Allocation A, B, C;
        Type.Builder builder = new Type.Builder(mRS, Element.U8(mRS));
        Type a_type = builder.setX(k).setY(m).create();
        Type b_type = builder.setX(k).setY(n).create();
        Type c_type = builder.setX(n).setY(m).create();

        A = Allocation.createTyped(mRS, a_type);
        B = Allocation.createTyped(mRS, b_type);
        C = Allocation.createTyped(mRS, c_type);

        A.copyFrom(a_byte);
        B.copyFrom(b_byte);
        // C doesn't matter, is output only

        ScriptIntrinsicBLAS blas = ScriptIntrinsicBLAS.create(mRS);
        blas.BGEMM(A, a_offset, B, b_offset, C, c_offset, c_mult_int);

        int c_count = (m * n);
        byte[] c_byte_output = new byte[c_count];
        C.copyTo(c_byte_output);
        return c_byte_output;
    }

    boolean testWithTolerance(byte[] c_byte, byte[] c_byte_output, String name) {
        final boolean areSizesDifferent = (c_byte.length != c_byte_output.length);
        final int c_count = Math.min(c_byte.length, c_byte_output.length);

        // The testing procedure here is a bit complex, but the aim is to mimic the
        // requirements we've empirically found running deep neural networks in real
        // applications. We want to open the door to vendors using approximations that
        // produce slightly different results for optimization's sake, but keep the
        // precision loss within small enough bounds that we don't lose accuracy in
        // the final result.
        // After experimentation, we've found that we can tolerate around 5% of the
        // output bytes being different by 1. Any larger differences are not tolerable
        // and we can't get good results if the frequency of small differences is
        // higher than 5%. This test tries to measure those properties on an example
        // set of parameters that were captured from a real application.
        // For example, if you uncommented this function that adds random noise to the
        // results at a 3% specified frequency, the test should fail:
        // addByteNoise(c_byte_output, c_count, 0.03f, 1);

        android.util.Log.e("8BGEMM", "Beginning " + name + " test compare.");
        int howManyDifferent = 0;
        boolean areAnyTooDifferent = false;
        for (int i = 0; i < c_count; i++) {
          byte expectedValue = c_byte[i];
          byte actualValue = c_byte_output[i];
          int delta = (expectedValue - actualValue);
          // First make sure that the difference is no more than one.
          if ((delta < -1) || (delta > 1)) {
              areAnyTooDifferent = true;
          }
          // If there is a difference, increment the counter to track it.
          if (delta != 0) {
              // Don't spam the logs if too many are different.
              if (howManyDifferent < 50) {
                  android.util.Log.e("8BGEMM", "Mismatch at " + i +
                          ": expected " + (expectedValue & 0xff) +
                          ", got " + (actualValue & 0xff));
              }
              ++howManyDifferent;
          }
        }
        // We want no more than 2% of the values to show any differences, so work out
        // what that means in absolute numbers.
        final int percentThreshold = 2;
        final int differenceThreshold = (percentThreshold * c_count) / 100;
        final boolean areTooManyDifferent = (howManyDifferent > differenceThreshold);
        if (areSizesDifferent) {
            android.util.Log.e("8BGEMM", "The number of values is different (" +
                   c_byte.length + " versus " + c_byte_output.length + ").");
        }
        if (areAnyTooDifferent) {
            android.util.Log.e("8BGEMM", "Some outputs were too different.");
        }
        if (areTooManyDifferent) {
            android.util.Log.e("8BGEMM", "There were too many small differences." +
                    " We can tolerate " + percentThreshold + "% (" +
                    differenceThreshold + "), but there were " + howManyDifferent);
        }
        boolean didFail = (areSizesDifferent || areAnyTooDifferent || areTooManyDifferent);
        if (didFail) {
            android.util.Log.e("8BGEMM", name + " testing failed!");
        } else {
            android.util.Log.e("8BGEMM", name + " testing passed!");
        }
        return !didFail;
    }

    private Bitmap loadBitmap(int resource) {
        final BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        return BitmapFactory.decodeResource(getResources(), resource, options);
    }
}
