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

package com.android.rs.test;

import android.content.Context;
import android.content.res.Resources;
import android.renderscript.*;
import android.os.SystemClock;

public class UT_shortlived_context extends UnitTest {
    private Resources mRes;
    private static int width = 20;
    private static int allocSize = width * width;

    private byte[] mInArray;
    private byte[] mOutArray;

    protected UT_shortlived_context(RSTestCore rstc, Resources res, Context ctx) {
        super(rstc, "Shortlived context", ctx);
        mRes = res;

        mInArray = new byte[allocSize * 4];
        mOutArray = new byte[allocSize * 4];
        for (int i=0; i < allocSize * 4; i++)
          mInArray[i] = (byte) (i % 256);
    }

    public void runOnce() {
        RenderScript pRS = RenderScript.create(mCtx);
        Element uchar4 = Element.U8_4(pRS);
        Type twoD = Type.createXY(pRS, Element.U8_4(pRS), width, width);

        Allocation inputAlloc = Allocation.createTyped(pRS, twoD);
        Allocation outputAlloc = Allocation.createTyped(pRS, twoD);
        inputAlloc.copyFrom(mInArray);

        ScriptIntrinsicBlur script = ScriptIntrinsicBlur.create(pRS, uchar4);
        script.setInput(inputAlloc);
        script.setRadius(4);

        script.forEach(outputAlloc);
        outputAlloc.copyTo(mOutArray);

        pRS.destroy();
    }

    public void run() {
        for (int iteration = 0; iteration < 100; iteration ++)
            runOnce();

        passTest();
    }
}
