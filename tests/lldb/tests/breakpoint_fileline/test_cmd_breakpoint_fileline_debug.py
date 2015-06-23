'''Module that contains the test TestCmdBreakpointFileLineDebug'''

from tests.test_base import TestBase

class TestCmdBreakpointFileLineDebug(TestBase):
    '''Tests the setting of a breakpoint on a specific line of a RS file,
       where the binary contains debug information.'''

    def __init__(self):
        TestBase.__init__(self)

    def get_name(self):
        '''return the test name'''
        return "test_cmd_breakpoint_fileline_debug"

    def get_bundle_targets(self):
        '''return string with name of bundle executable to run'''
        return ["JavaDebugWaitAttach"]

    def post_run(self):
        '''clean up after execution'''
        if self.platform_:
            self.platform_.DisconnectRemote()

    def test_case(self):
        '''Run the lldb commands that are being tested.'''
        self.try_command("language renderscript status",
                         ["Runtime Library discovered",
                          "Runtime Driver discovered"])

        self.try_command("breakpoint set --file simple.rs --line 8",
                         ["(pending)"])

        self.try_command("process continue",
                         [])

        self.try_command("bt",
                         ["librs.simple.so",
                          "simple_kernel",
                          "stop reason = breakpoint"])

        self.try_command("breakpoint list",
                         ["simple.rs",
                          "resolved = 1"])

        self.try_command("process status",
                         ["stopped",
                          "stop reason = breakpoint"])

        self.try_command(
            "language renderscript kernel breakpoint simple_kernel",
             ["simple_kernel",
              "within script",
              "simple",
              "Breakpoint(s) created"])

        self.try_command("breakpoint list",
                         [])

    def run(self, dbg, remote_pid, lldb):
        '''execute the actual test'''
        assert dbg
        assert remote_pid
        assert lldb

        self.dbg_ = dbg
        self.lldb_ = lldb

        try:
            self.test_assert(self.connect_to_platform(lldb, dbg, remote_pid))
            self.ci_ = dbg.GetCommandInterpreter()
            assert self.ci_

            self.test_assert(self.ci_.IsValid())
            self.test_assert(self.ci_.HasCommands())

            self.test_case()

        except self.TestFail:
            return False

        return True
