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

import android.renderscript.Int2;

/**
 * This class tests reduce kernels that are associative non-commutative functions.
 */
public class ReduceNonCommutativeTest extends RSBaseCompute {
    private ScriptC_reduce_non_commutative mScript;

    @Override
    public void setUp() throws Exception {
        super.setUp();
        mScript = new ScriptC_reduce_non_commutative(mRS);
    }

    /**
     * Tests a reduction involving a non-commutative (but associative) function.
     *
     * The function merge_intervals([a, b], [c, d]) takes two intervals [a, b], [c, d] as input, and
     * checks that a <= b and c <= d. If b == c, then merge_intervals() returns [a, d], otherwise if
     * b != c or if the intervals are invalid, it returns [-1, -1].
     */
    public void testReduceNonCommutative() {
        int SIZE = 20000;

        int[] input = new int[SIZE];
        // Fill with the pattern (0, 1), (1, 2), (2, 3), ...
        for (int i = 0; i < SIZE; ++i) {
            input[i] = (i - 1) / 2;
        }

        Int2 result = mScript.reduce_merge_intervals(input);

        assertEquals(0, result.x);
        assertEquals(SIZE / 2 - 1, result.y);
    }
}
