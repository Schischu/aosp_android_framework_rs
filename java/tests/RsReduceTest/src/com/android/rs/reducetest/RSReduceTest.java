/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.rs.reducetest;

import android.app.ListActivity;
import android.util.Log;
import android.os.Bundle;

import java.lang.Throwable;

/**
 * Main activity class for RsReduceTest. Based on RSTest.java.
 */
public class RSReduceTest extends ListActivity {
    private static final String LOG_TAG = "RsReduceTest";
    private static final boolean DEBUG  = true;
    private static final boolean LOG_ENABLED = true;

    private ReduceTestController mController;

    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);

        mController = new ReduceTestController(this);
        mController.init(this);
        RSReduceTest.log("should have seen this");
    }

    static void log(String message) {
        if (LOG_ENABLED) {
            Log.v(LOG_TAG, message);
        }
    }

    static void log(Throwable tr) {
        if (LOG_ENABLED) {
            Log.w(LOG_TAG, tr);
        }
    }
}
