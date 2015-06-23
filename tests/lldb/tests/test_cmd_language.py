'''Module that contains the test TestCmdLanguage'''

from .test_base import TestBase

class TestCmdLanguage(TestBase):
    '''Tests the "language" command and "language renderscript" subcommand
       without requiring a binary to be running.'''

    def __init__(self):
        TestBase.__init__(self)

    def get_name(self):
        '''return the test name'''
        return "test_cmd_language"

    def get_bundle_targets(self):
        '''return string with name of bundle executable to run'''
        return None

    def run(self, dbg, remote_pid, lldb):
        '''execute the actual test'''
        assert dbg

        cmd_interp = dbg.GetCommandInterpreter()

        if (not cmd_interp.IsValid() or
            not cmd_interp.HasCommands() or
            not cmd_interp.CommandExists("language")):
            return False

        res = lldb.SBCommandReturnObject()
        cmd_interp.HandleCommand("language", res)
        if not res.Succeeded():
            return False

        cmd_interp.HandleCommand("language renderscript", res)
        if not res.Succeeded():
            return False
        output = res.GetOutput()
        if not output:
            return False

        cmds = ["kernel", "context", "module", "status"]

        for cmd in cmds:
            if not cmd in output:
                return False

        return True
