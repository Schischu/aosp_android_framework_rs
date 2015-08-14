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

import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.renderscript.RSRuntimeException;
import android.renderscript.Script;
import android.renderscript.Type;

import android.util.Log;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * This test checks exceptional cases when launching reduce kernels.
 */
public class ReduceExceptionsTest extends RSBaseCompute {
    ScriptC_reduce_add mScript;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mScript = new ScriptC_reduce_add(mRS);
    }

    /**
     * Checks that passing input / output allocations of the wrong element type throws an error.
     */
    public void testElementTypeMismatch() {
        Allocation aBad = Allocation.createSized(mRS, Element.F32(mRS), 4);
        Allocation aGood = Allocation.createSized(mRS, Element.F32_4(mRS), 1);

        failReduce("passing ain of incorrect element type", "add_float4", aBad, aGood);
        failReduce("passing aout of incorrect element type", "add_float4", aGood, aBad);
    }

    /**
     * Checks that passing non-1D allocations as inputs throws an error.
     */
    public void testInputAllocationNot1D() {
        Allocation ain2x1 = Allocation.createTyped(mRS,
                Type.createXY(mRS, Element.F32(mRS), 2, 1));
        Allocation ain2x2x2 = Allocation.createTyped(mRS,
                Type.createXYZ(mRS, Element.F32(mRS), 2, 2, 2));
        Allocation aout = Allocation.createSized(mRS, Element.F32(mRS), 1);

        failReduce("reduce on 2x1 type", "add_float", ain2x1, aout);
        failReduce("reduce on 2x2x2 type", "add_float", ain2x2x2, aout);
    }

    /**
     * Checks that passing null input or output allocations is caught as an error.
     */
    public void testNullAllocation() {
        Allocation alloc = Allocation.createSized(mRS, Element.F32(mRS), 1);

        failReduce("passing null ain", "add_float", null, alloc);
        failReduce("passing null aout", "add_float", alloc, null);
    }

    /**
     * Checks that invalid bounds passed via LaunchOptions are caught as errors.
     */
    public void testInvalidBoundsToLaunchOptions() {
        /*
        TODO: Bug 23325998. This currently doesn't fail with an exception, but it should.

        final int size = 10;
        Allocation ain = Allocation.createSized(mRS, Element.F32(mRS), size);
        Allocation aout = Allocation.createSized(mRS, Element.F32(mRS), 1);

        failReduce("passing out of bounds LaunchOptions", "add_float", ain, aout,
                new Script.LaunchOptions().setX(0, 1 + size));
        */
    }

    /**
     * Checks that invalid bounds passed to the helper function are caught as errors.
     */
    public void testInvalidBoundsToHelper() {
        final int size = 10;
        float[] input = new float[size];

        failReduce("passing x1 = -1", "add_float4", input, -1, size);
        failReduce("passing x1 == x1", "add_float4", input, 1, 1);
        failReduce("passing x1 > x2", "add_float4", input, 2, 1);
        failReduce("passing x2 > length", "add_float4", input, 0, 1 + size);
    }

    /**
     * Checks that calling a vector helper function throws an error when the passed bounds are not
     * valid.
     */
    public void testVectorBoundsToVectorHelper() {
        failReduce("passing out of bounds range to add_float2", "add_float2", new float[2], 0, 2);
        failReduce("passing out of bounds range to add_float3", "add_float3", new float[3], 0, 2);
        failReduce("passing out of bounds range to add_float3", "add_float4", new float[4], 0, 2);
    }

    /**
     * Checks that passing an array without the correct amount of padding to a helper function of
     * vector type is caught as an error.
     */
    public void testIncorrectlyPaddedArrayToVectorHelper() {
        // add_float2()
        failReduce("passing array of length 1 to add_float2()", "add_float2", new float[1]);
        failReduce("passing array of length 1 to add_float2()", "add_float2", new float[1], 0, 1);

        // add_float3()
        failReduce("passing array of length 1 to add_float3()", "add_float3", new float[1]);
        failReduce("passing array of length 1 to add_float3()", "add_float3", new float[1], 0, 1);
        failReduce("passing array of length 2 to add_float3()", "add_float3", new float[2]);
        failReduce("passing array of length 2 to add_float3()", "add_float3", new float[2], 0, 1);

        // add_float4()
        failReduce("passing array of length 1 to add_float4()", "add_float4", new float[1]);
        failReduce("passing array of length 1 to add_float4()", "add_float4", new float[1], 0, 1);
        failReduce("passing array of length 2 to add_float4()", "add_float4", new float[2]);
        failReduce("passing array of length 2 to add_float4()", "add_float4", new float[2], 0, 1);
        failReduce("passing array of length 3 to add_float4()", "add_float4", new float[3]);
        failReduce("passing array of length 3 to add_float4()", "add_float4", new float[3], 0, 1);
    }

    /**
     * Checks that a null input passed to the helper function is caught as an error.
     */
    public void testNullInputToHelper() {
        failReduce("passing null array", "add_float", null);
        failReduce("passing null array", "add_float", null, 0, 1);
    }

    /**
     * Checks that an empty array passed to the helper function is caught as an error.
     */
    public void testEmptyInputToHelper() {
        failReduce("passing zero length array", "add_float", new float[0]);
        failReduce("passing zero length array", "add_float", new float[0], 0, 1);
    }

    /**
     * Calls the ScriptC_reduce_add method ending in methodSuffix, and checks that the call
     * generates an exception.
     */
    protected void expectRSException(String description, String methodSuffix,
            boolean isHelperVariant, boolean hasBounds, Object... args) {
        Method method = null;
        String methodName = "reduce_" + methodSuffix;
        try {
            if (isHelperVariant) {
                if (hasBounds) {
                    method = ScriptC_reduce_add.class.getMethod(
                            methodName, float[].class, int.class, int.class);
                } else {
                    method = ScriptC_reduce_add.class.getMethod(methodName, float[].class);
                }
            } else {
                if (hasBounds) {
                    method = ScriptC_reduce_add.class.getMethod(
                            methodName, Allocation.class, Allocation.class,
                            Script.LaunchOptions.class);
                } else {
                    method = ScriptC_reduce_add.class.getMethod(methodName, Allocation.class,
                            Allocation.class);
                }
            }
        } catch (Exception e) {
            Log.e("ReduceExceptionsTest", "Unable to get method", e);
            fail();
        }

        boolean sawException = false;
        try {
            method.invoke(mScript, args);
        } catch (InvocationTargetException e) {
            // Some calls will throw NullPointerException
            sawException = e.getCause() instanceof RSRuntimeException ||
                    e.getCause() instanceof NullPointerException;
        } catch (IllegalAccessException e) {
            Log.e("ReduceExceptionsTest", "Unable to call method", e);
            fail();
        }

        assertTrue(description, sawException);
    }

    protected void failReduce(String description, String methodSuffix, Allocation ain,
            Allocation aout) {
        expectRSException(description, methodSuffix, false, false, ain, aout);
    }

    protected void failReduce(String description, String methodSuffix, Allocation ain,
            Allocation aout, Script.LaunchOptions sc) {
        expectRSException(description, methodSuffix, false, true, ain, aout, sc);
    }

    protected void failReduce(String description, String methodSuffix, float[] in) {
        expectRSException(description, methodSuffix, true, false, in);
    }

    protected void failReduce(String description, String methodSuffix, float[] in, int x1,
            int x2) {
        expectRSException(description, methodSuffix, true, true, in, x1, x2);
    }
}
