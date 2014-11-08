#!/bin/bash

# Run the general RS CTS tests. We can expand this script to run more tests
# as we see fit, but this essentially should be a reasonable smoke test of
# RenderScript (to be run on build/test bots).

# We are currently in frameworks/rs, so compute our top-level directory.
MY_ANDROID_DIR=$PWD/../../
CTS_TRADEFED=$MY_ANDROID_DIR/out/host/linux-x86/bin/cts-tradefed

cd $MY_ANDROID_DIR

#$CTS_TRADEFED run commandAndExit cts --force-abi 64 -p android.renderscript
#$CTS_TRADEFED run commandAndExit cts --force-abi 32 -p android.renderscript
$CTS_TRADEFED run commandAndExit cts -p android.renderscript
exit $?
