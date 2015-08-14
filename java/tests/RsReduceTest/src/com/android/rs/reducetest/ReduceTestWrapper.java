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

package com.android.rs.reducetest;

import android.content.Context;
import android.util.Log;

import java.lang.reflect.Method;

import junit.framework.AssertionFailedError;
import junit.framework.Test;
import junit.framework.TestResult;
import junit.framework.TestListener;

/**
 * This class runs a test class in a thread and collects the results of the test.
 */
public class ReduceTestWrapper extends Thread {
    private ReduceTestController mController;
    private RSBaseCompute mReduceTest;
    private volatile boolean mPassed, mTestRun;

    public ReduceTestWrapper(RSBaseCompute reduceTest, ReduceTestController controller) {
        mController = controller;
        mReduceTest = reduceTest;
        mPassed = true;
        mTestRun = false;
    }

    static class WrapperListener implements TestListener {
        public void addError(Test test, Throwable t) {
            Log.e("ReduceTest", "woops", t);
        }
        public void addFailure(Test test, AssertionFailedError e) {
            Log.e("ReduceTest", "assert failed", e);
        }
        public void startTest(Test test) {}
        public void endTest(Test tesT) {}
    }

    public void run() {
        boolean failed = false;
        TestResult result = new TestResult();
        result.addListener(new WrapperListener());

        Method[] methods = mReduceTest.getClass().getMethods();
        for (Method m : methods) {
            if (m.getName().indexOf("test") != 0) continue;
            if (m.getReturnType() != Void.TYPE) continue;
            if (m.getParameterTypes().length != 0) continue;
            RSReduceTest.log(m.getName());

            mReduceTest.setName(m.getName());
            mReduceTest.run(result);

            mPassed &= result.wasSuccessful();
        }

        mTestRun = true;
        updateUI();
    }

    private void updateUI() {
        try {
            mController.refreshTestResults();
        }
        catch (IllegalStateException e) {
            /* Ignore the case where our message receiver has been
               disconnected. This happens when we leave the application
               before it finishes running all of the unit tests. */
        }
    }

    public String toString() {
        String out = mReduceTest.getClass().getSimpleName();
        if (!mTestRun) {
            return out;
        }
        if (mPassed) {
            out += " - PASSED";
        } else {
            out += " - FAILED";
        }
        return out;
    }
}
