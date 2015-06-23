- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
-   LLDB for Renderscript Test Suite
-
-   02/07/2015
- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

Overview:

    The LLDB for Renderscript test suite is written in python and relies on
    LLDB's python bindings.  The test suite will push several test app's onto
    a target device, and makes coordinated use of LLDB and ADB to run automated
    debug sessions.

Building the test suite:

    Checkout the AOSP and build it for your target. Navigate to
    /frameworks/rs/test/lldb and type mm.  This should successfully build the
    binaries that the testsuite uses. They will be placed in
    <path to out folder>/target/product/<product code name>/data/app.

Prerequisite:

    An lldb-server executable must be present on your device/emulator.
    LLDB must be compiled on your host machine along with its python interface.
    lldb-server and lldb should be built from the same source revisions.

Running the test suite:

    The test suite can be run via the following command:

        > python run_tests.py <config_file.py>

    An optional config file can be passed to the test suite which will provide
    details of your specific environment.  If no config file is passed then
    config_default.py will be used.

    The config interface should match config_default.py which can be taken as
    a starting point for modifications.

    If your config does not specify a path to the host lldb, the PYTHONPATH
    environment variable must be set.  The appropriate value to set this to can
    be obtained by running the following command:

        > lldb -P

    This will print out a path to the lldb python bindings on your local machine.

Build Requirements:

    This test suite currently only runs on Linux, as the LLDB Python bindings
    are unstable on windows.


    The following revisions are from the llvm git mirror:

    llvm : 76bee1a4507e0931e0790457e54d0ac6472f27c0
    clang: 4d058e4bed49934307bd591e40507212c8a2f0da
    lldb : b31bc788b0f1fa9ff46e99542d5d87fc3bdcbb23

    lldb has the following dependencies:

      Python2.7.6
      swig2.0
      lldb-server

Building LLDB python bindings:

    Build instructions for lldb can be found on the official lldb web page:

      http://lldb.llvm.org/build.html

    The following CMake variables should be enabled when generating:

      LLDB_ENABLE_PYTHON_SCRIPTS_SWIG_API_GENERATION = True

    As a post build step, swig will generate the python bindings for lldb.

A typical test transcript:

    Located ADB
    pushing JavaInfiniteLoop
    pushing JavaDebugWaitAttach
    Pushed all tests
    Pre run complete
    Found 5 tests
    running test_cmd_breakpoint_fileline_debug.py: pass
    running test_cmd_breakpoint_kernel_debug_2.py: pass
    running test_cmd_language_subcmds_debug.py: pass
    running test_cmd_language.py: pass
    running test_cmd_breakpoint_kernel_debug_1.py: pass
    5 of 5 passed
    100% rate
