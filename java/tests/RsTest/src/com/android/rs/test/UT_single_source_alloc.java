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

import java.lang.reflect.Method;

public class UT_single_source_alloc extends UnitTest {
    private Resources mRes;
    private int dimX = 3;
    private int dimY = 4;
    private int dimZ = 5;
    private int start = 23;

    // rs_data_type for float, double, char, short, int, long, uchar, ushort, uint, ulong in that
    // order
    private int rsDataTypes[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    protected UT_single_source_alloc(RSTestCore rstc, Resources res, Context ctx) {
        super(rstc, "SingleSourceAllocation", ctx);
        mRes = res;
    }

    private void initializeGlobals(RenderScript RS, ScriptC_single_source_alloc s) {
        s.set_gDimX(dimX);
        s.set_gDimY(dimY);
        s.set_gDimZ(dimZ);
        s.set_gStart(start);

        return;
    }

    public void run() {
        RenderScript pRS = RenderScript.create(mCtx);
        ScriptC_single_source_alloc s = new ScriptC_single_source_alloc(pRS);
        pRS.setMessageHandler(mRsMessage);

        initializeGlobals(pRS, s);

        // Test valid usages are accepted
        for (int dataType: rsDataTypes) {
            for (int vecSize = 1; vecSize <= 4; vecSize ++) {
                for (int nDims = 1; nDims <= 3; nDims ++) {
                    s.invoke_CreateAndTestAlloc(dataType, vecSize, nDims);
                }
            }
        }

        // Test invalid uses are rejected
        s.invoke_TestInvalidUses();

        s.invoke_single_source_alloc_test();
        pRS.finish();
        waitForMessage();
        pRS.destroy();
    }
}
