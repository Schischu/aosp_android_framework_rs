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
import android.renderscript.Script;

import java.lang.reflect.Method;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

/**
 * This test verifies that reduction works on supported primitive types.
 */
public class ReduceAddTest extends RSBaseCompute {
    private ScriptC_reduce_add mScript;

    public enum PrimitiveType {
        PRIMITIVE_bool,
        PRIMITIVE_char,
        PRIMITIVE_char2,
        PRIMITIVE_char3,
        PRIMITIVE_char4,
        PRIMITIVE_double,
        PRIMITIVE_double2,
        PRIMITIVE_double3,
        PRIMITIVE_double4,
        PRIMITIVE_float,
        PRIMITIVE_float2,
        PRIMITIVE_float3,
        PRIMITIVE_float4,
        PRIMITIVE_int,
        PRIMITIVE_int2,
        PRIMITIVE_int3,
        PRIMITIVE_int4,
        PRIMITIVE_long,
        PRIMITIVE_long2,
        PRIMITIVE_long3,
        PRIMITIVE_long4,
        PRIMITIVE_short,
        PRIMITIVE_short2,
        PRIMITIVE_short3,
        PRIMITIVE_short4,
        PRIMITIVE_uchar,
        PRIMITIVE_uchar2,
        PRIMITIVE_uchar3,
        PRIMITIVE_uchar4,
        PRIMITIVE_uint,
        PRIMITIVE_uint2,
        PRIMITIVE_uint3,
        PRIMITIVE_uint4,
        PRIMITIVE_ulong,
        PRIMITIVE_ulong2,
        PRIMITIVE_ulong3,
        PRIMITIVE_ulong4,
        PRIMITIVE_ushort,
        PRIMITIVE_ushort2,
        PRIMITIVE_ushort3,
        PRIMITIVE_ushort4
    }

    /**
     * Returns the name associated with a primitive type.
     * Eg. for PRIMITIVE_float4 returns "float4"
     */
    public static String getPrimitiveName(PrimitiveType type) {
        String name = type.name();
        return name.substring(1 + name.indexOf("_"));
    }

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mScript = new ScriptC_reduce_add(mRS);
    }

    /**
     * Small inputs test
     */
    public void testReduceAddSmallInputs() throws Exception {
        runReduceTestForAllTypes(1);
        runReduceTestForAllTypes(2);
        runReduceTestForAllTypes(3);
    }

    /**
     * Medium inputs test
     */
    public void testReduceAddMediumInputs() throws Exception {
        runReduceTestForAllTypes(1000);
    }

    /**
     * Large inputs test
     */
    public void testReduceAddLargeInputs() throws Exception {
        runReduceTestForAllTypes(1000000);
    }

    /**
     * Small inputs with clipped ranges
     */
    public void testReduceAddSmallInputs_clipped() throws Exception {
        final int SIZE = 6;
        for (int start = 0; start < SIZE; ++start) {
            for (int end = start + 1; end < SIZE; ++end) {
                runReduceTestForAllTypes(SIZE, start, end);
            }
        }
    }

    /**
     * Medium inputs with clipped ranges
     */
    public void testReduceAddMediumInputs_clipped() throws Exception {
        runReduceTestForAllTypes(1000, 0, 500);
        runReduceTestForAllTypes(1000, 1, 501);
        runReduceTestForAllTypes(1000, 500, 1000);
    }

    /**
     * Large inputs with clipped ranges
     */
    public void testReduceAddLargeInputs_clipped() throws Exception {
        runReduceTestForAllTypes(1000000, 34, 756459);
    }

    protected void runReduceTestForAllTypes(int allocationSize) throws Exception {
        runReduceTestForAllTypes(allocationSize, 0, allocationSize);
    }

    /**
     * Performs a test for all supported primitive types.
     *
     * For each primitive type, this creates an allocation of the specified size, initializes the
     * allocation, and then reduces the subrange x1 - x2.  The reduced value is checked against the
     * expected output.
     */
    protected void runReduceTestForAllTypes(int allocationSize, int x1, int x2) throws Exception {
        for (PrimitiveType type : PrimitiveType.values()) {
            Allocation ain = getInputAllocationWithPrimitiveType(type, allocationSize);
            Allocation aout = getAllocationWithPrimitiveType(type, 1);
            Method method = getReduceMethodForType(type);
            Script.LaunchOptions clip = new Script.LaunchOptions().setX(x1, x2);
            method.invoke(mScript, ain, aout, clip);
            checkAllocationAgainstExpectedOutput(aout, type, x1, x2);
        }
    }

    /**
     * Checks the output in the allocation aout against the expected output. The allocation should
     * be the result of calling reduce_add_* with the given type and subrange.
     */
    protected void checkAllocationAgainstExpectedOutput(Allocation aout, PrimitiveType type,
            int x1, int x2) throws Exception {
        getValidationMethodForType(type).invoke(mScript, aout, x1, x2);
        mScript.invoke_checkError();
        waitForMessage();
        checkForErrors();
    }

    protected Method getReduceMethodForType(PrimitiveType type) throws Exception {
        return ScriptC_reduce_add.class.getMethod(
                "reduce_add_" + getPrimitiveName(type),
                new Class[]{Allocation.class, Allocation.class, Script.LaunchOptions.class});
    }

    protected Method getSetUpMethodForType(PrimitiveType type) throws Exception {
        return ScriptC_reduce_add.class.getMethod(
                "invoke_set_up_input_" + getPrimitiveName(type),
                new Class[]{Allocation.class});
    }

    protected Method getValidationMethodForType(PrimitiveType type) throws Exception {
        return ScriptC_reduce_add.class.getMethod(
                "invoke_verify_output_" + getPrimitiveName(type),
                new Class[]{Allocation.class, long.class, long.class});
    }

    /**
     * Returns and initializes a 1D Allocation of the specified primitive element type, having the
     * specified size.
     */
    protected Allocation getInputAllocationWithPrimitiveType(PrimitiveType type, int size)
            throws Exception {
        Allocation alloc = getAllocationWithPrimitiveType(type, size);
        getSetUpMethodForType(type).invoke(mScript, alloc);
        return alloc;
    }

    /**
     * Returns a 1D Allocation of the specified primitive element type, having the specified size.
     */
    protected Allocation getAllocationWithPrimitiveType(PrimitiveType type, int size)
            throws Exception {
        // Get the element type based on the primitive type name.
        Pattern typePattern = Pattern.compile("(u?)([a-z]*)([0-9]?)");
        Matcher match = typePattern.matcher(getPrimitiveName(type));
        assertTrue(match.matches());
        String unsigned = match.group(1);
        String baseType = match.group(2);
        String vecElems = match.group(3);

        String sign = unsigned.equals("u") ? "U" : "I";

        String shortName = "";
        if (baseType.equals("bool")) {
            shortName = "BOOLEAN";
        } else if (baseType.equals("char")) {
            shortName = sign + "8";
        } else if (baseType.equals("short")) {
            shortName = sign + "16";
        } else if (baseType.equals("int")) {
            shortName = sign + "32";
        } else if (baseType.equals("long")) {
            shortName = sign + "64";
        } else if (baseType.equals("float")) {
            shortName = "F32";
        } else if (baseType.equals("double")) {
            shortName = "F64";
        } else {
            fail("unknown base type: " + baseType);
        }

        if (!vecElems.equals("")) {
            shortName += "_" + vecElems;
        }

        Element elem = (Element) Element.class
                .getMethod(shortName, new Class[]{RenderScript.class})
                .invoke(null, mRS);

        return Allocation.createSized(mRS, elem, size);
    }
}
