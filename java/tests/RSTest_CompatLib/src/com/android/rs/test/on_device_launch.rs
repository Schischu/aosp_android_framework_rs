#include "shared.rsh"

int dimX;
int dimY;
/*
static bool failed = false;


rs_allocation ain, aout;

void root(const int *in, int *out, const void *usrData, uint32_t x, uint32_t y) {
    rsForEach(1, ain, aout);
    *out = *in * 2;
}
*/
int __attribute__((kernel)) goo(const int a, uint32_t x, uint32_t y) {
    // return a + x + y * dimX;
    return a + 1;
}

int __attribute__((kernel)) foo(int a) {
    return a * 2;
}

int __attribute__((kernel)) bar(int a) {
    return a + 41;
}

static void validate(rs_allocation out) {
    bool failed = false;

    int i, j;

    for (j = 0; j < dimY; j++) {
        for (i = 0; i < dimX; i++) {
            const int actual = rsGetElementAt_int(out, i, j);
            //const int expected = (i + j * dimX) * 2 + 41;
            // const int expected = (i + j * dimX) * dimX * dimY * 2;
            //const int expected = dimX * dimY * 2;
	    const int expected = (i + j * dimX) * 2;
            if (actual != expected) {
                failed = true;
                rsDebug("row    ", j);
                rsDebug("column ", i);
                rsDebug("expects", expected);
                rsDebug("got    ", actual);
            }
        }
    }

    if (failed) {
        rsDebug("FAILED", 0);
    } else {
        rsDebug("PASSED", 0);
    }

    if (failed) {
        rsSendToClientBlocking(RS_MSG_TEST_FAILED);
    } else {
        rsSendToClientBlocking(RS_MSG_TEST_PASSED);
    }

}

void entrypoint(rs_allocation in, rs_allocation out) {
    int i, j;
    for (i = 0; i < dimX; i++) {
        for (j = 0; j < dimY; j++) {
            rsSetElementAt_int(in, j * dimX + i, i, j);
        }
    }

/*
    ain = in;
    aout = out;
*/
    rsParallelFor(foo, in, out);
#if 0
    rsForEach(1, in, out);
    rsForEach(2, out, out);
    rsForEach(3, out, out);
#endif

    validate(out);
}
