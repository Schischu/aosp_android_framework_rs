from test_base import test_base

# Tests the "language" command and "language renderscript" subcommand without requiring a binary to be running.
class test_cmd_language(test_base):

    # return the test name
    def get_name(self):
        return "test_cmd_language"

    # return string with name of bundle executable to run
    def get_bundle_targets(self):
        return None

    # execute the actual test
    def run(self, dbg, remote_pid, lldb):
        assert dbg

        ci = dbg.GetCommandInterpreter()
        assert ci

        if not ci.IsValid():
            return False

        if not ci.HasCommands():
            return False

        if not ci.CommandExists("language"):
            return False

        res = lldb.SBCommandReturnObject()
        ci.HandleCommand("language", res)
        if not res.Succeeded():
            return False

        ci.HandleCommand("language renderscript", res)
        if not res.Succeeded():
            return False
        output = res.GetOutput()
        if not output:
            return False

        cmds = ["kernel", "context", "module", "status"]

        for x in cmds:
            if output.find(x) == -1:
                return False

        return True
