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

package com.android.rs.image;

import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.renderscript.Script;
import android.renderscript.ScriptIntrinsicConvolve3x3;
import android.renderscript.ScriptIntrinsicColorMatrix;
import android.renderscript.Type;
import android.renderscript.Matrix4f;
import android.renderscript.ScriptGroup;
import android.renderscript.ScriptGroup2;
import android.renderscript.Closure;
import android.renderscript.UnboundValue;
import android.util.Log;

import java.lang.Math;
import java.util.HashMap;

public class GroupTest extends TestBase {
    private ScriptIntrinsicConvolve3x3 mConvolve;
    private ScriptIntrinsicColorMatrix mMatrix;

    private Allocation mScratchPixelsAllocation1;
    private ScriptGroup mGroup;
    private ScriptGroup2 mGroup2;

    private int mWidth;
    private int mHeight;
    private int mMode;

    public static final int EMULATED = 0;
    public static final int NATIVE1 = 1;
    public static final int NATIVE2 = 2;

    public GroupTest(int mode) {
        mMode = mode;
    }

    public void createTest(android.content.res.Resources res) {
        mWidth = mInPixelsAllocation.getType().getX();
        mHeight = mInPixelsAllocation.getType().getY();

        mConvolve = ScriptIntrinsicConvolve3x3.create(mRS, Element.U8_4(mRS));
        mMatrix = ScriptIntrinsicColorMatrix.create(mRS, Element.U8_4(mRS));

        float f[] = new float[9];
        f[0] =  0.f;    f[1] = -1.f;    f[2] =  0.f;
        f[3] = -1.f;    f[4] =  5.f;    f[5] = -1.f;
        f[6] =  0.f;    f[7] = -1.f;    f[8] =  0.f;
        mConvolve.setCoefficients(f);

        Matrix4f m = new Matrix4f();
        m.set(1, 0, 0.2f);
        m.set(1, 1, 0.9f);
        m.set(1, 2, 0.2f);
        mMatrix.setColorMatrix(m);

        Type.Builder tb = new Type.Builder(mRS, Element.U8_4(mRS));
        tb.setX(mWidth);
        tb.setY(mHeight);
        Type connect = tb.create();

        switch (mMode) {
          case NATIVE1:
            ScriptGroup.Builder b = new ScriptGroup.Builder(mRS);
            b.addKernel(mConvolve.getKernelID());
            b.addKernel(mMatrix.getKernelID());
            b.addConnection(connect, mConvolve.getKernelID(), mMatrix.getKernelID());
            mGroup = b.create();
            break;
          case NATIVE2:
            ScriptGroup2.Builder b2 = new ScriptGroup2.Builder(mRS);
            UnboundValue in = b2.addInput();
            HashMap<Script.FieldID, Object> bindings = new HashMap<Script.FieldID, Object>();
            bindings.put(mConvolve.getFieldID_Input(), in);
            Closure c1 = b2.addKernel(mConvolve.getKernelID(), connect,
                new Object[0], bindings);
            Closure c2 = b2.addKernel(mMatrix.getKernelID(),
                mOutPixelsAllocation.getType(), new Object[]{c1.getReturn()},
                new HashMap<Script.FieldID, Object>());
            mGroup2 = b2.create(c2.getReturn());
            break;
          case EMULATED:
            mScratchPixelsAllocation1 = Allocation.createTyped(mRS, connect);
            break;
        }
    }

    public void runTest() {
        switch (mMode) {
          case NATIVE1:
            mConvolve.setInput(mInPixelsAllocation);
            mGroup.setOutput(mMatrix.getKernelID(), mOutPixelsAllocation);
            mGroup.execute();
            break;
          case NATIVE2:
            mOutPixelsAllocation = (Allocation)mGroup2.execute(mInPixelsAllocation)[0];
            break;
          case EMULATED:
            mConvolve.setInput(mInPixelsAllocation);
            mConvolve.forEach(mScratchPixelsAllocation1);
            mMatrix.forEach(mScratchPixelsAllocation1, mOutPixelsAllocation);
            break;
        }
    }

}
