/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 */

package com.android.rs.test;

import android.content.Context;
import android.content.res.Resources;
import android.renderscript.*;
import android.util.Log;
import java.lang.Thread;
import java.util.HashMap;

public class UT_sg2_gatherscatter extends UnitTest {
    private Resources mRes;

    private static final int ARRAY_SIZE = 256;

    private static final String TAG = "ScriptGroup2 (GatherScatter)";

    int[] mArray;

    protected UT_sg2_gatherscatter(RSTestCore rstc, Resources res, Context ctx) {
        super(rstc, TAG, ctx);
        mRes = res;
    }

    public void initializeGlobals(RenderScript RS, ScriptC_addup s) {
        mArray = new int[ARRAY_SIZE * 4];

        for (int i = 0; i < ARRAY_SIZE; i++) {
            mArray[i*4] = i * 7;
            mArray[i*4 + 1] = i * 7;
            mArray[i*4 + 2] = i * 7;
            mArray[i*4 + 3] = i * 7;
        }
    }

    public void run() {
        RenderScript pRS = RenderScript.create(mCtx);
        ScriptC_addup s = new ScriptC_addup(pRS);
        pRS.setMessageHandler(mRsMessage);
        initializeGlobals(pRS, s);

        Allocation input = Allocation.createSized(pRS, Element.I32_4(pRS), ARRAY_SIZE);
        input.copyFrom(mArray);

        ScriptGroup2.Builder builder = new ScriptGroup2.Builder(pRS);

        HashMap<Script.FieldID, Object> map = new HashMap<Script.FieldID, Object>();

        ScriptGroup2.UnboundValue unbound = builder.addInput();

        ScriptGroup2.Closure c = null;
        ScriptGroup2.Future f = null;
        int stride;
        for (stride = ARRAY_SIZE / 2; stride >= 1; stride >>= 1) {
            map.put(s.getFieldID_reduction_stride(), new Integer(stride));
            if (f == null) {
                map.put(s.getFieldID_a_in(), unbound);
            } else {
                map.put(s.getFieldID_a_in(), f);
            }
            c = builder.addKernel(s.getKernelID_add(),
                                  Type.createX(pRS, Element.I32_4(pRS), stride),
                                  new Object[0],
                                  map);
            f = c.getReturn();
        }

        ScriptGroup2 group = builder.create(c.getReturn());

        if (c == null) {
            return;
        }

        int[] a = new int[4];
        ((Allocation)group.execute(input)[0]).copyTo(a);

        pRS.finish();
        pRS.destroy();

        boolean failed = false;
        for (int i = 0; i < 4; i++) {
            if (failed == false && a[i] != ARRAY_SIZE * (ARRAY_SIZE - 1) * 7 / 2) {
                Log.e(TAG, "a["+i+"]="+a[i]+", should be "+ (ARRAY_SIZE * (ARRAY_SIZE - 1) * 7 / 2));
                failed = true;
            }
        }
        if (failed) {
            failTest();
            return;
        }
        passTest();
    }
}
