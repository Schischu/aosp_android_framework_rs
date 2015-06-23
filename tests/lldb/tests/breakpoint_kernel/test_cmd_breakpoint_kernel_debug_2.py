from tests.test_base import test_base

class test_fail:
    def __init__(self):
        pass

def test_assert(cond):
    if not cond:
        raise test_fail()

# Tests the setting of a breakpoint on a RS kernel, where the binary contains debug information.
class test_cmd_breakpoint_kernel_debug_2(test_base):

    # return the test name
    def get_name(self):
        return "test_cmd_breakpoint_kernel_debug"

    # return string with name of bundle executable to run
    def get_bundle_targets(self):
        return ["JavaInfiniteLoop"]

    # clean up after execution
    def post_run(self):
        if self.platform_:
            self.platform_.DisconnectRemote()

    # run an lldb command an match expected response
    def try_command(self, cmd, expected):
        assert self.lldb_
        assert self.ci_
        test_assert(self.find_in_output(self.lldb_, self.ci_, cmd, expected))

    # execute the actual test
    def test_case(self):

        self.try_command("language",
                         [])

        self.try_command("b simple_kernel",
                         [""])

        self.try_command("breakpoint list",
                         ["simple_kernel",
                          "resolved"])

        self.try_command("process continue",
                         ["resuming",
                          "stopped",
                          "stop reason = breakpoint"])

        self.try_command("bt",
                         ["stop reason = breakpoint",
                          "frame #0:",
                          "librs.infiniteloop.so",
                          "infiniteLoop.rs:30",
                          "simple_kernel"])

        self.try_command("breakpoint list",
                         ["resolved = 1"])

        self.try_command("process status",
                         ["stopped",
                          ".so`simple_kernel",
                          "stop reason = breakpoint"])

    # execute the actual test
    def run(self, dbg, remote_pid, lldb):
        assert dbg
        assert remote_pid
        assert lldb

        self.dbg_  = dbg
        self.lldb_ = lldb

        try:
            test_assert(self.connect_to_platform(lldb, dbg, remote_pid))
            self.ci_ = dbg.GetCommandInterpreter()
            assert self.ci_

            test_assert(self.ci_.IsValid())
            test_assert(self.ci_.HasCommands())

            self.test_case()

        except test_fail:
            return False

        return True