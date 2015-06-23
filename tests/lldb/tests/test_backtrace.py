'''Module that contains the test TestBacktrace'''

from harness.test_base import TestBase

#
class TestBacktrace(TestBase):
    '''Tests breaking on a kernel and a function
       then viewing the call stack.'''

    def __init__(self, config):
        TestBase.__init__(self, config)

    def get_name(self):
        '''return the test name

        Returns:
            String representing the name of the test
		'''
        return 'test_cmd_backtrace_function'

    def get_bundle_target(self):
        '''return string with name of bundle executable to run

        Returns:
            List of strings containing the names of the binaries that this test
            can be run with.
        '''
        return 'JavaFunction'

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

        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('b simple_kernel',
                          ['Breakpoint 1',
                           '(pending)'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('bt',
                         ['stop reason = breakpoint',
                          # We should be able to see three functions in bt:
                          # libRSCpuRef,kernel.expand, and the kernel
                          'frame #2:',
                          'librs.function.so',
                          'function.rs:14',
                          'simple_kernel'])

        self.try_command('breakpoint delete 1', ['1 breakpoints deleted'])

        self.try_command('b getColour',
                         ['Breakpoint 2',
                          'getColour'])

        self.try_command('breakpoint list',
                         ['getColour', 'resolved'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('bt',
                         ['stop reason = breakpoint',
                          # We should be able to see four functions in bt:
                          # libRSCpuRef, kernel.expand, kernel, and the function
                          'frame #3:',
                          'librs.function.so',
                          'function.rs:9',
                          'getColour'])



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
