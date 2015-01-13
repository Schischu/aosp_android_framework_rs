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

        Allocation A, B, C;
        Type.Builder builder = new Type.Builder(mRS, Element.F32(mRS));
        Type matrix = builder.setX(2).setY(2).create();
        A = Allocation.createTyped(mRS, matrix);
        B = Allocation.createTyped(mRS, matrix);
        C = Allocation.createTyped(mRS, matrix);
        
        ScriptC_blas blashelper = new ScriptC_blas(mRS);
        blashelper.set_a(A);
        blashelper.set_b(B);
        blashelper.set_c(C);
        
        blashelper.invoke_setup();

        blashelper.invoke_dump(A);
        blashelper.invoke_dump(B);
        blashelper.invoke_dump(C);

        ScriptIntrinsicBLAS blas = ScriptIntrinsicBLAS.create(mRS);
        blas.SGEMM(0, 0, 1.f, A, B, 1.f, C);
        
        blashelper.invoke_dump(C);
    }

    private Bitmap loadBitmap(int resource) {
        final BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        return BitmapFactory.decodeResource(getResources(), resource, options);
    }
}
