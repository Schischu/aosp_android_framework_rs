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
import java.util.HashMap;

public class UT_closure2 extends UnitTest {
  private Resources mRes;

  private static final int ARRAY_SIZE = 2;

  private static final String TAG = "Closure Test 2";

  protected UT_closure2(RSTestCore rstc, Resources res, Context ctx) {
    super(rstc, TAG, ctx);
    mRes = res;
  }

  public void run() {
    RenderScript pRS = RenderScript.create(mCtx);
    ScriptC_closure1 s = new ScriptC_closure1(pRS);
    pRS.setMessageHandler(mRsMessage);

    int[] a0 = new int[ARRAY_SIZE * 4];

    for (int i = 0; i < ARRAY_SIZE; i++) {
      a0[i*4] = i;
      a0[i*4 + 1] = i;
      a0[i*4 + 2] = i;
      a0[i*4 + 3] = i;
    }

    Allocation input = Allocation.createSized(pRS, Element.I32_4(pRS), ARRAY_SIZE);
    input.copyFrom(a0);

    ScriptGroup2.Builder builder = new ScriptGroup2.Builder(pRS);

    HashMap<Script.FieldID, Object> map = new HashMap<Script.FieldID, Object>();

    UnboundValue unbound = builder.addInput();

    Closure c = null;
    Future f = null;
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
          null,
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
      Log.i(TAG, "a["+i+"]="+a[i]+", should be "+ (ARRAY_SIZE * (ARRAY_SIZE - 1) / 2));
      if (failed == false && a[i] != ARRAY_SIZE * (ARRAY_SIZE - 1) / 2) {
        failed = true;
      }
    }

    for (int i = 0; i < ARRAY_SIZE; i++) {
      a0[i*4] = 1;
      a0[i*4 + 1] = 1;
      a0[i*4 + 2] = 1;
      a0[i*4 + 3] = 1;
    }

    // input.copyFrom(a0);
    Allocation input1 = Allocation.createSized(pRS, Element.I32_4(pRS), ARRAY_SIZE);
    input1.copyFrom(a0);

    ((Allocation)group.execute(input1)[0]).copyTo(a);

    for (int i = 0; i < 4; i++) {
      Log.i(TAG, "a["+i+"]="+a[i]+", should be "+ ARRAY_SIZE);
      if (failed == false && a[i] != ARRAY_SIZE) {
        failed = true;
      }
    }

    pRS.finish();
    pRS.destroy();

    if (failed) {
      failTest();
      return;
    }
    passTest();
  }
}
