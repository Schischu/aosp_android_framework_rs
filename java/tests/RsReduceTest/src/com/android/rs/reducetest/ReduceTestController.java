/*
 * Copyright (C) 2008-2012 The Android Open Source Project
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

import android.content.Context;
import android.content.res.Resources;
import android.renderscript.*;
import android.util.Log;
import java.util.ArrayList;
import java.util.ListIterator;
import java.util.Timer;
import java.util.TimerTask;
import android.app.ListActivity;
import android.widget.ArrayAdapter;

/**
 * Main logic to drive the execution of the reduce tests.
 * Based on RSTestCore.java.
 */
public class ReduceTestController {
    ListActivity mCtx;

    public ReduceTestController(ListActivity ctx) {
        mCtx = ctx;
    }

    private ArrayList<ReduceTestWrapper> unitTests;
    private ListIterator<ReduceTestWrapper> test_iter;
    private ReduceTestWrapper activeTest;
    private boolean stopTesting;

    private ArrayAdapter<ReduceTestWrapper> testAdapter;

    /* Periodic timer for ensuring future tests get scheduled */
    private Timer mTimer;
    public static final int RS_TIMER_PERIOD = 100;

    public void init(Context ctx) {
        stopTesting = false;

        unitTests = new ArrayList<ReduceTestWrapper>();

        unitTests.add(new ReduceTestWrapper(
            new ReduceAddTest().setUpContext(ctx), this));

        testAdapter = new ArrayAdapter<ReduceTestWrapper>(mCtx,
            android.R.layout.simple_list_item_1, unitTests);
        mCtx.setListAdapter(testAdapter);

        test_iter = unitTests.listIterator();
        refreshTestResults(); /* Kick off the first test */

        TimerTask pTask = new TimerTask() {
            public void run() {
                refreshTestResults();
            }
        };

        mTimer = new Timer();
        mTimer.schedule(pTask, RS_TIMER_PERIOD, RS_TIMER_PERIOD);
    }

    public void checkAndRunNextTest() {
        mCtx.runOnUiThread(new Runnable() {
                public void run() {
                    if (testAdapter != null)
                        testAdapter.notifyDataSetChanged();
                }
            });

        if (activeTest != null) {
            if (!activeTest.isAlive()) {
                /* Properly clean up on our last test */
                try {
                    activeTest.join();
                }
                catch (InterruptedException e) {
                }
                activeTest = null;
            }
        }

        if (!stopTesting && activeTest == null) {
            if (test_iter.hasNext()) {
                activeTest = test_iter.next();
                activeTest.start();
                /* This routine will only get called once when a new test
                 * should start running. The message handler in ReduceTestWrapper.java
                 * ensures this. */
            }
            else {
                if (mTimer != null) {
                    mTimer.cancel();
                    mTimer.purge();
                    mTimer = null;
                }
            }
        }
    }

    public void refreshTestResults() {
        checkAndRunNextTest();
    }

    public void cleanup() {
        stopTesting = true;
        ReduceTestWrapper t = activeTest;

        /* Stop periodic refresh of testing */
        if (mTimer != null) {
            mTimer.cancel();
            mTimer.purge();
            mTimer = null;
        }

        /* Wait to exit until we finish the current test */
        if (t != null) {
            try {
                t.join();
            }
            catch (InterruptedException e) {
            }
            t = null;
        }

    }
}
