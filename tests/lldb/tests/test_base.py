'''Module that contains TestBase, the base class of all tests.'''

from config import Config

class TestBase(object):
    '''Base class for all tests. Provides some common functionality.'''

    platform_ = None    # platform
    target_ = None      # target
    process_ = None     # process
    cmd_ = None         # command interpreter
    listener_ = None    # event listener
    log_ = []           # output log
    lldb_ = None        # handle to the lldb module
    dbg_ = None         # instance of the debugger for this test
    ci_ = None          # instance of the lldb command interpreter for this test

    def __init__(self):
        pass

    def get_name(self):
        '''return the test name'''
        raise NotImplementedError

    def get_bundle_targets(self):
        '''return string with name of bundle executable to run'''
        raise NotImplementedError

    def run(self, dbg, remote_pid, lldb):
        '''execute the actual test'''
        raise NotImplementedError

    def post_run(self):
        '''clean up after'''
        pass

    def get_log(self):
        '''get the lldb log'''
        if self.log_ is None:
            return []
        return self.log_

    def connect_to_platform(self, lldb_module, dbg, remote_pid):
        '''connect to an lldb platform that has been started elsewhere'''
        if type(remote_pid) is int:
            remote_pid = str(remote_pid)

        err1 = dbg.SetCurrentPlatform("remote-android")
        if err1.Fail():
            return False

        self.platform_ = dbg.GetSelectedPlatform()
        if not self.platform_:
            return False

        connect_string = "connect://localhost:{0}"\
            .format(Config.get_device_port())
        opts = lldb_module.SBPlatformConnectOptions(connect_string)
        err2 = self.platform_.ConnectRemote(opts)
        if err2.Fail():
            return False

        self.target_ = dbg.CreateTarget(None)
        if not self.target_:
            return False

        dbg.SetSelectedTarget(self.target_)
        self.listener_ = lldb_module.SBListener()
        err3 = lldb_module.SBError()
        self.process_ = self.target_.AttachToProcessWithID(self.listener_,
                                                           int(remote_pid),
                                                           err3)
        if err3.Fail() or not self.process_:
            return False

        return True

    def log_append(self, cmd):
        '''Append a string to the log.'''
        if self.log_ is None:
            self.log_ = []
        self.log_.append(cmd)

    def find_in_output(self, lldb, cmd_interp, cmd, strings_to_find):
        '''Runs an lldb command and
           checks the output for a series of strings.'''
        res = lldb.SBCommandReturnObject()

        self.log_append(cmd)

        cmd_interp.HandleCommand(cmd, res)
        if not res.Succeeded():
            print "Command \"" + cmd + "\" failed."
            print " The output was: \"" + res.GetError() + "\""
            return False

        output = res.GetOutput()
        if not output:
            print "Command \"" + cmd + "\" had no output."
            return False

        self.log_append(output)

        for string in strings_to_find:
            if output.find(string) == -1:
                print "Expected to find \"" + string + "\" in the output."
                print " Found: \"" + output + "\""
                return False

        return True

    class TestFail(Exception):
        '''An exception that is thrown when a line in a test fails,
           i.e. a lldb command does not return the expected string.'''
        pass

    def test_assert(self, cond):
        '''Check a given condition and raise the TestFail exception
           if it is False.'''
        if not cond:
            raise self.TestFail()

    def try_command(self, cmd, expected):
        '''run an lldb command an match expected response'''
        assert self.lldb_
        assert self.ci_
        self.test_assert(self.find_in_output(self.lldb_,
                                             self.ci_,
                                             cmd,
                                             expected))
