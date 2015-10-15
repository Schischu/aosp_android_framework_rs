#include "shared.rsh"

int gDimX;
int gDimY;
int gDimZ;
int gStart;
static bool failed = false;

rs_allocation gAlloc;

// For each type, define 4 kernels, one per vector variant, that walk an
// allocation and validate each cell.  The value in a cell must be gStart +
// "index-of-the-cell-starting-from-zero".  For vector types, the 'x' field
// must have this value.  The expected values for 'y', 'z' and 'w' follow the
// 'x' value in increments of one.
//
// 'z' will be zero for 2D and 1D allocations.  'y' will be zero for 1D
// allocations.

// TODO When the requirement that kernels must return an output to be launched
// using rsParallelFor is relaxed, make the kernel not return its input.
#define VERIFY_KERNEL(CT)                                                      \
    CT RS_KERNEL verify_##CT(CT in, int x, int y, int z) {                     \
        int val = (gStart + x + y * gDimX + z * gDimY * gDimX);                \
        _RS_ASSERT_EQU(in, (CT) val);                                          \
        return in;                                                             \
    }                                                                          \
    CT##2 RS_KERNEL verify_##CT##2(CT##2 in, int x, int y, int z) {            \
        int val = (gStart + x + y * gDimX + z * gDimY * gDimX);                \
        _RS_ASSERT_EQU(in.x, (CT) val);                                        \
        _RS_ASSERT_EQU(in.y, (CT) (val + 1));                                  \
        return in;                                                             \
    }                                                                          \
    CT##3 RS_KERNEL verify_##CT##3(CT##3 in, int x, int y, int z) {            \
        int val = (gStart + x + y * gDimX + z * gDimY * gDimX);                \
        _RS_ASSERT_EQU(in.x, (CT) val);                                        \
        _RS_ASSERT_EQU(in.y, (CT) (val + 1));                                  \
        _RS_ASSERT_EQU(in.z, (CT) (val + 2));                                  \
        return in;                                                             \
    }                                                                          \
    CT##4 RS_KERNEL verify_##CT##4(CT##4 in, int x, int y, int z) {            \
        int val = (gStart + x + y * gDimX + z * gDimY * gDimX);                \
        _RS_ASSERT_EQU(in.x, (CT) val);                                        \
        _RS_ASSERT_EQU(in.y, (CT) (val + 1));                                  \
        _RS_ASSERT_EQU(in.z, (CT) (val + 2));                                  \
        _RS_ASSERT_EQU(in.w, (CT) (val + 3));                                  \
        return in;                                                             \
    }                                                                          \

VERIFY_KERNEL(float)
VERIFY_KERNEL(double)
VERIFY_KERNEL(char)
VERIFY_KERNEL(short)
VERIFY_KERNEL(int)
VERIFY_KERNEL(long)
VERIFY_KERNEL(uchar)
VERIFY_KERNEL(ushort)
VERIFY_KERNEL(uint)
VERIFY_KERNEL(ulong)


// Store to an allocation based on the vector size and dimensionality being
// tested.  SETLEMENTAT, STORE_TO_ALLOC capture the following variables from
// the context where they get instantiated:
//     vecSize, numDims, gAlloc, val, x, y, z

#define SETELEMENTAT(CT)                                                      \
    if (numDims == 3) {                                                       \
        rsSetElementAt_##CT(gAlloc, storeVal, x, y, z);                       \
    }                                                                         \
    else if (numDims == 2) {                                                  \
        rsSetElementAt_##CT(gAlloc, storeVal, x, y);                          \
    }                                                                         \
    else {                                                                    \
        rsSetElementAt_##CT(gAlloc, storeVal, x);                             \
    }

#define STORE_TO_ALLOC(RST, CT)                                               \
    case RST:                                                                 \
        switch (vecSize) {                                                    \
            case 1: {                                                         \
                CT storeVal = (CT) val;                                       \
                SETELEMENTAT(CT);                                             \
                     }                                                        \
                break;                                                        \
            case 2: {                                                         \
                CT##2 storeVal = {(CT) val, (CT) (val + 1)};                  \
                SETELEMENTAT(CT##2);                                          \
                    }                                                         \
                break;                                                        \
            case 3: {                                                         \
                CT##3 storeVal = {(CT) val, (CT) (val + 1), (CT) (val + 2)};  \
                SETELEMENTAT(CT##3);                                          \
                    }                                                         \
                break;                                                        \
            case 4: {                                                         \
                CT##4 storeVal = {(CT) val, (CT) (val + 1), (CT) (val + 2),   \
                                  (CT) (val + 3)};                            \
                SETELEMENTAT(CT##4);                                          \
                    }                                                         \
                break;                                                        \
        }                                                                     \
        break;                                                                \


// Launch the verify_kernel based on the appropriate type and vector size.
#define LAUNCH_VERIFY_KERNEL(RST, CT)                                         \
    case RST:                                                                 \
        if (vecSize == 1) rsParallelFor(verify_##CT, gAlloc, gAlloc);         \
        else if (vecSize == 2) rsParallelFor(verify_##CT##2, gAlloc, gAlloc); \
        else if (vecSize == 3) rsParallelFor(verify_##CT##3, gAlloc, gAlloc); \
        else if (vecSize == 4) rsParallelFor(verify_##CT##4, gAlloc, gAlloc); \
        break;

void CreateAndTestAlloc(int dataType, int vecSize, int numDims) {
    rs_data_type dt = (rs_data_type) dataType;

    // Create the allocation
    if (vecSize > 1) {
        if (numDims == 3)
            gAlloc = rsCreateVectorAllocation(dt, vecSize, gDimX, gDimY, gDimZ);
        else if (numDims == 2)
            gAlloc = rsCreateVectorAllocation(dt, vecSize, gDimX, gDimY);
        else
            gAlloc = rsCreateVectorAllocation(dt, vecSize, gDimX);
    }
    else {
        if (numDims == 3)
            gAlloc = rsCreatePrimitiveAllocation(dt, gDimX, gDimY, gDimZ);
        else if (numDims == 2)
            gAlloc = rsCreatePrimitiveAllocation(dt, gDimX, gDimY);
        else
            gAlloc = rsCreatePrimitiveAllocation(dt, gDimX);
    }

    if (!rsIsObject(gAlloc))
        return;

    for (int z = 0; z < gDimZ; z ++) {
        for (int y = 0; y < gDimY; y ++) {
            for (int x = 0; x < gDimX; x ++) {
                int val = gStart + (x + y * gDimX + z * gDimY * gDimX);

                // Store to a cell based on the type, vector size and
                // dimensionality
                switch (dt) {
                    STORE_TO_ALLOC(RS_TYPE_FLOAT_32, float)
                    STORE_TO_ALLOC(RS_TYPE_FLOAT_64, double)
                    STORE_TO_ALLOC(RS_TYPE_SIGNED_8, char)
                    STORE_TO_ALLOC(RS_TYPE_SIGNED_16, short)
                    STORE_TO_ALLOC(RS_TYPE_SIGNED_32, int)
                    STORE_TO_ALLOC(RS_TYPE_SIGNED_64, long)
                    STORE_TO_ALLOC(RS_TYPE_UNSIGNED_8, uchar)
                    STORE_TO_ALLOC(RS_TYPE_UNSIGNED_16, ushort)
                    STORE_TO_ALLOC(RS_TYPE_UNSIGNED_32, uint)
                    STORE_TO_ALLOC(RS_TYPE_UNSIGNED_64, ulong)
                    default: break;
                }
            }
            // Do not process y > 0 for 1-D allocations
            if (numDims == 1) break;
        }
        // Do not process z > 0 for 1-D and 2-D allocations
        if (numDims <= 2) break;
    }

    // Launch the appropriate verify_ kernel
    switch (dt) {
        LAUNCH_VERIFY_KERNEL(RS_TYPE_FLOAT_32, float)
        LAUNCH_VERIFY_KERNEL(RS_TYPE_FLOAT_64, double)
        LAUNCH_VERIFY_KERNEL(RS_TYPE_SIGNED_8, char)
        LAUNCH_VERIFY_KERNEL(RS_TYPE_SIGNED_16, short)
        LAUNCH_VERIFY_KERNEL(RS_TYPE_SIGNED_32, int)
        LAUNCH_VERIFY_KERNEL(RS_TYPE_SIGNED_64, long)
        LAUNCH_VERIFY_KERNEL(RS_TYPE_UNSIGNED_8, uchar)
        LAUNCH_VERIFY_KERNEL(RS_TYPE_UNSIGNED_16, ushort)
        LAUNCH_VERIFY_KERNEL(RS_TYPE_UNSIGNED_32, uint)
        LAUNCH_VERIFY_KERNEL(RS_TYPE_UNSIGNED_64, ulong)

        default: break;
    }
}

void single_source_alloc_test() {
    if (failed) {
        rsSendToClientBlocking(RS_MSG_TEST_FAILED);
    }
    else {
        rsSendToClientBlocking(RS_MSG_TEST_PASSED);
    }
}
