from config import config

# Base class for all tests. Provides some common functionality.
class test_base():

    platform_ = None # platform
    target_   = None # target
    process_  = None # process
    cmd_      = None # command interpreter
    listener_ = None # event listener
    log_      = []   # output log

    # return the test name
    def get_name(self):
        assert False and "should not get here"
        return "test_base"

    # return string with name of bundle executable to run
    def get_bundle_targets(self):
        return None

    # execute the actual test
    def run(self, dbg, remote_pid, lldb):
        return True

    # get the lldb log
    def get_log(self):
        if self.log_ is None:
            return []
        return self.log_

    # connect to an lldb platform that has been started elsewhere
    def connect_to_platform(self, lldb_module, dbg, remote_pid):
        if type(remote_pid) is int:
            remote_pid = str(remote_pid)

        err1 = dbg.SetCurrentPlatform("remote-android")
        if err1.Fail():
            return False

        self.platform_ = dbg.GetSelectedPlatform()
        if not self.platform_:
            return False

        opts = lldb_module.SBPlatformConnectOptions("connect://localhost:{0}".format(config.get_device_port()))
        err2 = self.platform_.ConnectRemote(opts)
        if err2.Fail():
            return False

        self.target_ = dbg.CreateTarget(None)
        if not self.target_:
            return False

        dbg.SetSelectedTarget(self.target_)
        self.listener_ = lldb_module.SBListener()
        err3 = lldb_module.SBError()
        self.process_ = self.target_.AttachToProcessWithID(self.listener_, int(remote_pid), err3)
        if err3.Fail() or not self.process_:
            return False

        return True

    # clean up after
    def post_run(self):
        pass

    def log_append(self, cmd):
        if self.log_ is None:
            self.log_ = []
        self.log_.append(cmd)

    # Runs an lldb command and checks the output for a series of strings.
    def find_in_output(self, lldb, ci, cmd, strings_to_find):
        res = lldb.SBCommandReturnObject()

        self.log_append(cmd)

        ci.HandleCommand(cmd, res)
        if not res.Succeeded():
            print "Command \"" + cmd + "\" failed. The output was: \"" + res.GetError() + "\""
            return False

        output = res.GetOutput()
        if not output:
            print "Command \"" + cmd + "\" had no output."
            return False

        self.log_append(output)

        for x in strings_to_find:
            if output.find(x) == -1:
                print "Expected to find \"" + x + "\" in the output. Found: \"" + output + "\""
                return False

        return True
