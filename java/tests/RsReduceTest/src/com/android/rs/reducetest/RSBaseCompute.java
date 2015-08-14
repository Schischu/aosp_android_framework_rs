/*
 * Copyright (C) 2010 The Android Open Source Project
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
import android.renderscript.RenderScript;
import android.renderscript.RenderScript.RSErrorHandler;
import android.renderscript.RenderScript.RSMessageHandler;
import android.renderscript.RSRuntimeException;
import android.test.AndroidTestCase;
import android.util.Log;

import junit.framework.TestCase;

/**
 * This class is based on android.renderscript.cts.RSBaseCompute. It is intended as a stand-in so
 * that subclasses of this class can be copied over to CTS with a minimum of changes.
 */
public class RSBaseCompute extends TestCase {
    Context mCtx;
    RenderScript mRS;

    /**
     * Passes a Context to the test class.
     *
     * This is done through a setter, rather than by a constructor, so that subclasses do not need
     * to have a constructor, as in the CTS tests.
     */
    public RSBaseCompute setUpContext(Context context) {
        mCtx = context;
        return this;
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mRS = RenderScript.create(mCtx);
        mRS.setMessageHandler(mRsMessage);
    }

    @Override
    protected void tearDown() throws Exception {
        if (mRS != null) {
            mRS.destroy();
            mRS = null;
        }
        super.tearDown();
    }

    // msgHandled is used to synchronize between waitForMessage() and the
    // RSMessageHandler thread.
    private volatile boolean msgHandled;

    private int result;
    private static final int RS_MSG_TEST_PASSED = 100;
    private static final int RS_MSG_TEST_FAILED = 101;

    RSMessageHandler mRsMessage = new RSMessageHandler() {
        public void run() {
            if (result == 0) {
                switch (mID) {
                    case RS_MSG_TEST_PASSED:
                    case RS_MSG_TEST_FAILED:
                        result = mID;
                        break;
                    default:
                        fail("Got unexpected RS message");
                        return;
                }
            }
            msgHandled = true;
        }
    };

    /**
     * Wait until we receive a message from the script object.
     */
    protected void waitForMessage() {
        while (!msgHandled) {
            Thread.yield();
        }
    }

    /**
     * Verify that we didn't fail on the control or script side of things.
     */
    protected void checkForErrors() {
        assertFalse(FoundError);
        assertTrue(result != RS_MSG_TEST_FAILED);
    }

    protected boolean FoundError = false;
    protected RSErrorHandler mRsError = new RSErrorHandler() {
        public void run() {
            FoundError = true;
            Log.e("RenderscriptCTS", mErrorMessage);
            throw new RSRuntimeException("Received error " + mErrorNum +
                                         " message " + mErrorMessage);
        }
    };
}
