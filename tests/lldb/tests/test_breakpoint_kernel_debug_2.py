'''Module that contains the test TestBreakpointKernelDebug2'''

from harness.test_base import TestBase

#
class TestBreakpointKernelDebug2(TestBase):
    '''Tests the setting of a breakpoint on a RS kernel,
       where the binary contains debug information.'''

    def __init__(self, config):
        TestBase.__init__(self, config)

    def get_name(self):
        '''return the test name

        Returns:
            String representing the name of the test
		'''
        return 'test_breakpoint_kernel_debug_2'

    def get_bundle_target(self):
        '''return string with name of bundle executable to run

        Returns:
            List of strings containing the names of the binaries that this test
            can be run with.
        '''
        return 'JavaInfiniteLoop'

    def post_run(self):
        '''clean up after execution'''
        if self.platform_:
            self.platform_.DisconnectRemote()

    def test_case(self):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
            output.
        '''
        self.try_command('language',
                         [])

        self.try_command('b simple_kernel',
                         [''])

        self.try_command('breakpoint list',
                         ['simple_kernel',
                          'resolved'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('bt',
                         ['stop reason = breakpoint',
                          'frame #0:',
                          'librs.infiniteloop.so',
                          'infiniteLoop.rs:28',
                          'simple_kernel'])

        self.try_command('breakpoint list',
                         ['resolved = 1'])

        self.try_command('process status',
                         ['stopped',
                          '.so`simple_kernel',
                          'stop reason = breakpoint'])

    def run(self, dbg, remote_pid, lldb):
        '''execute the actual test

        Args:
            dbg: The instance of the SBDebugger that is used to test commands.
            remote_pid: The integer that is the process id of the binary that
                the debugger is attached to.
            lldb: A handle to the lldb module.

        Returns:
            True if the test passed, or False if not.
        '''
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
