from tests.test_base import test_base

class test_fail:
    def __init__(self):
        pass

def test_assert(cond):
    if not cond:
        raise test_fail()

# Tests the "language renderscript" "kernel", "context", "module" and "status" subcommands on binaries with debug info
class test_cmd_language_subcmds_debug(test_base):

    # return the test name
    def get_name(self):
        return "test_cmd_language_subcmds_debug"

    # return string with name of bundle executable to run
    def get_bundle_targets(self):
        return ["JavaDebugWaitAttach"]

    # clean up after execution
    def post_run(self):
        if self.platform_:
            self.platform_.DisconnectRemote()

    # run an lldb command an match expected response
    def try_command(self, cmd, expected):
        assert self.lldb_
        assert self.ci_
        test_assert(self.find_in_output(self.lldb_, self.ci_, cmd, expected))

    # the main test case
    def test_case(self):

        self.try_command("language",
                         [])

        self.try_command("language renderscript status",
                         ["Runtime Library discovered",
                          "Runtime Driver discovered"])

        self.try_command("breakpoint set --file simple.rs --line 8",
                         ["(pending)"])

        self.try_command("process continue",
                         [])

        self.try_command("language renderscript kernel",
                         ["breakpoint",
                          "list"])

        self.try_command("language renderscript kernel list",
                         ["RenderScript Kernels",
                          "Resource",
                          "root"])

        self.try_command("language renderscript context",
                         ["dump"])

        self.try_command("language renderscript context dump",
                         ["Inferred RenderScript Contexts"])

        self.try_command("language renderscript module",
                         ["dump",
                          "probe"])

        self.try_command("language renderscript module dump",
                         ["RenderScript Modules:",
                          "Debug info loaded",
                          "Globals:",
                          "Kernels:",
                          "java_package_name:",
                          "version:"])

        self.try_command("language renderscript status",
                         ["Runtime Library discovered",
                          "Runtime Driver discovered",
                          "Runtime functions hooked",
                          "rsdAllocationInit",
                          "rsdAllocationRead2D",
                          "rsdScriptInit",
                          "rsdScriptInvokeForEach",
                          "rsdScriptInvokeForEachMulti",
                          "rsdScriptInvokeFunction",
                          "rsdScriptSetGlobalVar"])

    # execute the actual test
    def run(self, dbg, remote_pid, lldb):
        assert dbg
        assert remote_pid
        assert lldb

        self.lldb_ = lldb
        self.dbg_  = dbg

        try:
            if not self.connect_to_platform(lldb, dbg, remote_pid):
                return False
            self.ci_ = dbg.GetCommandInterpreter()
            assert self.ci_

            test_assert(self.ci_.IsValid())
            test_assert(self.ci_.HasCommands())
            self.test_case()

        except test_fail:
            return False

        return True
