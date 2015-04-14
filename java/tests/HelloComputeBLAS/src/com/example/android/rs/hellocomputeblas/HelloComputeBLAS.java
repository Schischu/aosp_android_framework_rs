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
    }

    static {
        System.loadLibrary("8bgemm");
    }

    native void getData(byte[] a, byte[] b, byte[] c);

    private void AddByteNoise(byte[] data, int count, float frequency, int maxDelta) {
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
        byte[] c_byte_output = new byte[c_count];

        getData(a_byte, b_byte, c_byte);

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

        C.copyTo(c_byte_output);

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
        // AddByteNoise(c_byte_output, c_count, 0.03f, 1);

        android.util.Log.e("8BGEMM", "Beginning compare");
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
        final boolean areTooManyDifferent = (howManyDifferent >= differenceThreshold);
        if (areAnyTooDifferent) {
            android.util.Log.e("8BGEMM", "Some outputs were too different.");
        }
        if (areTooManyDifferent) {
            android.util.Log.e("8BGEMM", "There were too many small differences." +
                    " We can tolerate " + percentThreshold + "% (" +
                    differenceThreshold + "), but there were " + howManyDifferent);
        }
        if (areAnyTooDifferent || areTooManyDifferent) {
            android.util.Log.e("8BGEMM", "Testing failed!");
        } else {
            android.util.Log.e("8BGEMM", "Testing passed!");
        }
    }

    private Bitmap loadBitmap(int resource) {
        final BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        return BitmapFactory.decodeResource(getResources(), resource, options);
    }
}
