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
      for (int i = 0; i < input.length; ++i){
        // We'll wrap around if the inputs are outside 0 to 255.
        if (input[i] < 128) {
          output[i] = (byte)(input[i]);
        } else {
          output[i] = (byte)(-(input[i] - 128));

        }
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
        final int c_rows = 4;
        final int c_cols = 2;
        final int c_count = (c_rows * c_cols);

        final int m = a_cols;
        final int n = b_cols;
        final int k = a_rows;

        byte[] c_byte_output = runGemm(m, n, k, a_data, a_offset, b_data, b_offset,
                c_offset, c_mult_int);
        testWithTolerance(expected_data, c_byte_output, "small matrices");
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
                        ": expected " + expectedValue +
                        ", got " + actualValue);
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
