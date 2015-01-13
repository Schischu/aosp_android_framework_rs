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
        android.util.Log.e("8BGEMM", "Beginning compare");
        for (int i = 0; i < c_count; i++) {
            if (c_byte[i] != c_byte_output[i]) {
                android.util.Log.e("8BGEMM", "Mismatch at " + i + ": expected " + c_byte[i] + ", got " + c_byte_output[i]);
                //                break;
            }
        }
        android.util.Log.e("8BGEMM", "Testing passed!");

        // compare C to c_byte

        /*
          blashelper.invoke_dump(C);

        Type.Builder builder2 = new Type.Builder(mRS, Element.F32(mRS));
        Type matrixT = builder.setX(15).setY(10).create();
        Type matrixT2 = builder.setX(42).setY(10).create();
        Type matrixT3 = builder.setX(42).setY(15).create();

        A = Allocation.createTyped(mRS, matrixT);
        B = Allocation.createTyped(mRS, matrixT2);
        C = Allocation.createTyped(mRS, matrixT3);

        blashelper.set_a(A);
        blashelper.set_b(B);
        blashelper.set_c(C);

        blashelper.invoke_setup();

        blashelper.invoke_dump(A);
        mRS.finish();
        android.util.Log.e("HATS", "A done");
        blashelper.invoke_dump(B);
        mRS.finish();
        android.util.Log.e("HATS", "B done");
        blashelper.invoke_dump(C);
        mRS.finish();
        android.util.Log.e("HATS", "C done");

        blas.SGEMM(ScriptIntrinsicBLAS.TRANSPOSE, ScriptIntrinsicBLAS.NO_TRANSPOSE, 1.f, A, B, 1.f, C);

        blashelper.invoke_dump(C);
        */
    }

    private Bitmap loadBitmap(int resource) {
        final BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        return BitmapFactory.decodeResource(getResources(), resource, options);
    }
}
