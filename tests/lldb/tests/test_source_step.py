'''Module that contains the test TestSourceStep'''

from harness.test_base import TestBase

class TestSourceStep(TestBase):

    def __init__(self, config):
        TestBase.__init__(self, config)

    def get_name(self):
        '''return the test name

        Returns:
            String representing the name of the test
        '''
        return 'test_source_step'

    def get_bundle_target(self):
        '''return string with name of bundle executable to run

        Returns:
            A string containing the name of the binary that this test
            can be run with.
        '''
        return "KernelScalarVariables"

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

        self.try_command('b -f scalars.rs -l 47',
                         ['(pending)'])

        self.try_command('process continue',
                         ['stopped',
                          'stop reason = breakpoint',
                          'scalars.rs:47',
                          '-> 47',
                          'int i = in;'])
        #47     int i = in;
        self.try_command('thread step-in',
                         ['-> 48'])
        #48     float f = (float) i;
        self.try_command('thread step-in',
                         ['-> 49'])
        #49     modify_f(&f);
        self.try_command('thread step-over',
                         ['-> 50'])
        #50  	modify_i(&i);
        self.try_command('thread step-in',
                         ['-> 33'])
        #33         int j = *i;
        self.try_command('b -f scalars.rs -l 38',
                         ['librs.scalars.so`modify_i',
                          'scalars.rs:38'])
        self.try_command('c',
                         ['stop reason = breakpoint',
                          'scalars.rs:38',
                          '-> 38'])
        #38    set_i(i, 0);
        self.try_command('thread step-in',
                         ['-> 22'])
        #22    int tmp = b;
        self.try_command('thread step-out',
                         ['-> 38'])
        return

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
