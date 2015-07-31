'''Module that contains the test TestReadScalars'''

from harness.test_base import TestBase

class TestReadScalar(TestBase):
    '''Tests inspecting local variables of all types'''

    def __init__(self, config):
        TestBase.__init__(self, config)

    def get_name(self):
        '''return the test name

        Returns:
            String representing the name of the test
        '''
        return 'test_read_scalar'

    def get_bundle_target(self):
        '''return string with name of bundle executable to run

        Returns:
            A string containing the name of the binary that this test
            can be run with.
        '''
        return "KernelScalarVariables"

    def post_run(self):
        '''clean up after execution'''
        if self.platform_:
            self.platform_.DisconnectRemote()

    def try_inspecting_local(self, local_name, expected_output):
        '''Run the "expr" and "frame variable" commands on a given local and
        with a given output. (The commands should be equivalent.)

        Args:
            local_name: String which is the name of the global to inspect
            expected_output: List of strings that should be found in the output

        Raises:
            TestFail: One of the lldb commands did not provide the expected
            output.
        '''
        self.try_command('expr ' + local_name, expected_output)
        self.try_command('frame variable ' + local_name, expected_output)

    def test_case(self):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
            output.
        '''

        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('breakpoint set --file scalars.rs --line 47', ['pending'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('frame variable',
                         ['(int) in',
                          '(int) i',
                          '(float) f',
                          '(int) ret'])

        self.try_inspecting_local('ret', ['(int)'])

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
