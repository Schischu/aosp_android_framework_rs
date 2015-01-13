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
    rsSetElementAt_float(a, 0, 0, 1.f);
    rsSetElementAt_float(a, 0, 1, 0.f);
    rsSetElementAt_float(a, 1, 0, 0.f);
    rsSetElementAt_float(a, 1, 1, 1.f);
    rsSetElementAt_float(b, 0, 0, 1.f);
    rsSetElementAt_float(b, 0, 1, 0.f);
    rsSetElementAt_float(b, 1, 0, 0.f);
    rsSetElementAt_float(b, 1, 1, 1.f);
    rsSetElementAt_float(c, 0, 0, 1.f);
    rsSetElementAt_float(c, 0, 1, 0.f);
    rsSetElementAt_float(c, 1, 0, 0.f);
    rsSetElementAt_float(c, 1, 1, 1.f);
}

void dump(rs_allocation hats) {
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++) {
            float3 f;
            f.x = i;
            f.y = j;
            f.z = rsGetElementAt_float(hats, i, j);
            rsDebug("matrix dump", f);
        }
            
}
