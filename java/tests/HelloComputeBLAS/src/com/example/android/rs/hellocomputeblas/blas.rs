/*
 * Copyright (C) 2011 The Android Open Source Project
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

#pragma version(1)
#pragma rs java_package_name(com.example.android.rs.hellocomputeblas)

rs_allocation a;
rs_allocation b;
rs_allocation c;

void setup() {
    int counter = 1;
    rsDebug("A X", rsAllocationGetDimX(a));
    rsDebug("A Y", rsAllocationGetDimY(a));
    for (int i = 0; i < rsAllocationGetDimY(a); i++) {
        for (int j = 0; j < rsAllocationGetDimX(a); j++) {
            rsSetElementAt_float(a, counter++, j, i);
        }
    }
    for (int i = 0; i < rsAllocationGetDimY(b); i++) {
        for (int j = 0; j < rsAllocationGetDimX(b); j++) {
            rsSetElementAt_float(b, counter++, j, i);
        }
    }
    for (int i = 0; i < rsAllocationGetDimY(c); i++) {
        for (int j = 0; j < rsAllocationGetDimX(c); j++) {
            rsSetElementAt_float(c, 0.f, j, i);
        }
    }
}

void dump(rs_allocation hats) {
    rsDebug("hats X", rsAllocationGetDimX(hats));
    rsDebug("hats Y", rsAllocationGetDimY(hats));
    for (int i = 0; i < rsAllocationGetDimY(hats); i++)
        for (int j = 0; j < rsAllocationGetDimX(hats); j++) {
            float3 f;
            f.x = j;
            f.y = i;
            f.z = rsGetElementAt_float(hats, j, i);
            rsDebug("matrix dump", f);
        }
            
}
