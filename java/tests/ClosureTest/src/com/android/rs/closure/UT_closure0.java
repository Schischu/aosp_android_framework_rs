/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.android.rs.closure;

import android.content.Context;
import android.content.res.Resources;
import android.renderscript.*;
import android.util.Log;
import java.lang.Thread;
import java.util.HashMap;

public class UT_closure0 extends UnitTest {
  private Resources mRes;

  private static final int ARRAY_SIZE = 256;

  private static final String TAG = "Closure Test 0";

  protected UT_closure0(RSTestCore rstc, Resources res, Context ctx) {
    super(rstc, TAG, ctx);
    mRes = res;
  }

  public void run() {
    RenderScript pRS = RenderScript.create(mCtx);
    ScriptC_increment s_inc = new ScriptC_increment(pRS);
    ScriptC_doubble s_double = new ScriptC_doubble(pRS);
    pRS.setMessageHandler(mRsMessage);

    int[] array = new int[ARRAY_SIZE * 4];

    for (int i = 0; i < ARRAY_SIZE * 4; i++) {
      array[i] = i;
    }

    Allocation input = Allocation.createSized(pRS, Element.I32_4(pRS), ARRAY_SIZE);
    input.copyFrom(array);

    ScriptGroup2.Builder builder = new ScriptGroup2.Builder(pRS);

    HashMap<Script.FieldID, Object> map = new HashMap<Script.FieldID, Object>();

    UnboundValue unbound = builder.addInput();

    Closure c0 = builder.addKernel(s_inc.getKernelID_increment(),
        Type.createX(pRS, Element.I32_4(pRS), ARRAY_SIZE),
        new Object[]{unbound}, map);

    Closure c1 = builder.addKernel(s_double.getKernelID_doubble(),
        Type.createX(pRS, Element.I32_4(pRS), ARRAY_SIZE),
        new Object[]{c0.getReturn()}, map);

    Closure c2 = builder.addKernel(s_double.getKernelID_doubble(),
        Type.createX(pRS, Element.I32_4(pRS), ARRAY_SIZE),
        new Object[]{c1.getReturn()}, map);

    ScriptGroup2 group = builder.create(c2.getReturn());

    int[] a = new int[ARRAY_SIZE * 4];
    ((Allocation)group.execute(input)[0]).copyTo(a);

    pRS.finish();
    pRS.destroy();

    boolean failed = false;
    for (int i = 0; i < ARRAY_SIZE * 4; i++) {
      if (a[i] != (i+1) * 4) {
        Log.e(TAG, "a["+i+"]="+a[i]+", should be "+ ((i+1) * 4));
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
