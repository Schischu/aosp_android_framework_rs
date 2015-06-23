'''Module that contains the test TestCmdLanguage'''

from harness.test_base import TestBase

class TestCmdLanguage(TestBase):
    '''Tests the "language" command and "language renderscript" subcommand
       without requiring a binary to be running.'''

    def __init__(self, config):
        TestBase.__init__(self, config)

    def get_name(self):
        '''return the test name'''
        return 'test_cmd_language'

    def get_bundle_target(self):
        '''return string with name of bundle executable to run'''
        return None

    def run(self, dbg, _, lldb):
        '''execute the actual test

        Args:
            dbg: The instance of the SBDebugger that is used to test commands.
            lldb: A handle to the lldb module.

        Returns:
            True if the test passed, or False if not.
        '''
        assert dbg

        cmd_interp = dbg.GetCommandInterpreter()

        if (not cmd_interp.IsValid() or
            not cmd_interp.HasCommands() or
            not cmd_interp.CommandExists('language')):
            return False

        res = lldb.SBCommandReturnObject()
        cmd_interp.HandleCommand('language', res)
        if not res.Succeeded():
            return False

        cmd_interp.HandleCommand('language renderscript', res)
        if not res.Succeeded():
            return False
        output = res.GetOutput()
        if not output:
            return False

        cmds = ['kernel', 'context', 'module', 'status']

        for cmd in cmds:
            if not cmd in output:
                return False

        return True
