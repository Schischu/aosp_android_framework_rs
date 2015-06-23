- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
-   LLDB for Renderscript Test Suite
-
-   23/06/2015
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

Running the test suite:

    The test suite requires some setup steps before it can be run.  A path to
    the LLDB python bindings must be exported as an environment variable so
    python can find them.

    running, "lldb -P" will give you the locations of the bindings.
    On my machine this gives:

      aidan@aidan-VirtualBox ~/rs/build/bin $ ./lldb-3.7.0 -P
      /home/aidan/rs/build/bin/../lib/python2.7/site-packages

    It is exported as follows:

      export PYTHONPATH="/home/aidan/rs/build/bin/../lib/python2.7/site-packages"

    Push an lldb-server executable to your device/emulator. Open the file
    rs/tests/lldb/config.py and modify it to contain paths local to your
    machine. Connect a device you wish to debug on or start the emulator.
    The test suite can then be executed using the command:

      python run.py

Requirements:

    This test suite currently only runs on Linux, as the LLDB Python bindings
    are unstable on windows.


    The following revisions are from the llvm git mirror:

    llvm : 544d686bc0d6e10f483106829e4f7a69346d4b26
    lldb : efc91053b816d4a2d020c47287fb02888d5f26e0
    clang: c024327365be5c472f3a1afd222161f6aa4ee2d6

    Supplied is "arm-fix.patch" which contains several fixes for ARM targets.
    LLDB and LLDB-Server must be built from the same patched source.

    lldb has the following dependancies:

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

    pushing JavaInfiniteLoop
    pushing JavaDebugWaitAttach
    running:
      test_cmd_language: pass
      test_cmd_language_subcmds_debug:JavaDebugWaitAttach: pass
      test_cmd_breakpoint_kernel_debug:JavaDebugWaitAttach: pass
      test_cmd_breakpoint_kernel_debug:JavaInfiniteLoop: pass
      test_cmd_breakpoint_fileline_debug:JavaDebugWaitAttach: pass
    5 of 5 passed: 100%

