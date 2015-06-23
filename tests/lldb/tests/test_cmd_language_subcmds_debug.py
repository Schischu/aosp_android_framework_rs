'''Module that contains the test TestCmdLanguageSubcmdsDebug'''

from harness.test_base import TestBase

class TestCmdLanguageSubcmdsDebug(TestBase):
    '''Tests the 'language renderscript' 'kernel', 'context', 'module'
       and 'status' subcommands on binaries with debug info'''

    def __init__(self, config):
        TestBase.__init__(self, config)

    def get_name(self):
        '''return the test name

        Returns:
            String representing the name of the test
        '''
        return 'test_cmd_language_subcmds_debug'

    def get_bundle_target(self):
        '''return string with name of bundle executable to run

        Returns:
            String containing the name of the binary that this test
            can be run with.
        '''
        return 'JavaDebugWaitAttach'

    def post_run(self):
        '''clean up after execution'''
        if self.platform_:
            self.platform_.DisconnectRemote()

    def test_case(self):
        '''Run the lldb commands that are being tested.'''
        self.try_command('language',
                         [])

        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('breakpoint set --file simple.rs --line 8',
                         ['(pending)'])

        self.try_command('process continue',
                         [])

        self.try_command('language renderscript kernel',
                         ['breakpoint',
                          'list'])

        self.try_command('language renderscript kernel list',
                         ['RenderScript Kernels',
                          'Resource',
                          'root'])

        self.try_command('language renderscript context',
                         ['dump'])

        self.try_command('language renderscript context dump',
                         ['Inferred RenderScript Contexts'])

        self.try_command('language renderscript module',
                         ['dump',
                          'probe'])

        self.try_command('language renderscript module dump',
                         ['RenderScript Modules:',
                          'Debug info loaded',
                          'Globals:',
                          'Kernels:',
                          'java_package_name:',
                          'version:'])

        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered',
                          'Runtime functions hooked',
                          'rsdAllocationInit',
                          'rsdAllocationRead2D',
                          'rsdScriptInit',
                          'rsdScriptInvokeForEach',
                          'rsdScriptInvokeForEachMulti',
                          'rsdScriptInvokeFunction',
                          'rsdScriptSetGlobalVar'])

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

        self.lldb_ = lldb
        self.dbg_ = dbg

        try:
            if not self.connect_to_platform(lldb, dbg, remote_pid):
                return False
            self.ci_ = dbg.GetCommandInterpreter()
            assert self.ci_

            self.test_assert(self.ci_.IsValid())
            self.test_assert(self.ci_.HasCommands())
            self.test_case()

        except self.TestFail:
            return False

        return True
