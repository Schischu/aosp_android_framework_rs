'''Module that contains TestBase, the base class of all tests.'''

from config import Config
from log import LOG

class TestBase(object):
    '''Base class for all tests. Provides some common functionality.'''

    platform_ = None # platform
    target_ = None # target
    process_ = None # process
    cmd_ = None # command interpreter
    listener_ = None # event listener
    log_ = [] # output log
    lldb_ = None # handle to the lldb module
    dbg_ = None # instance of the debugger for this test
    ci_ = None # instance of the lldb command interpreter for this test

    def __init__(self):
        pass

    def get_name(self):
        '''return the test name

        Returns:
            String representing the name of the test
        '''
        raise NotImplementedError

    def get_bundle_targets(self):
        '''return string with name of bundle executable to run

        Returns:
            List of strings containing the names of the binaries that this test
            can be run with.
        '''
        raise NotImplementedError

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
        raise NotImplementedError

    def post_run(self):
        '''clean up after'''
        pass

    def get_log(self):
        '''get the lldb log

        Returns:
            A string which is the contents of the log.
        '''
        if self.log_ is None:
            return []
        return self.log_

    def connect_to_platform(self, lldb_module, dbg, remote_pid):
        '''connect to an lldb platform that has been started elsewhere

        Args:
            lldb_module: A handle to the lldb module.
            dbg: The instance of the SBDebugger that should connect to the
                server.
            remote_pid: The integer that is the process id of the binary that
                the debugger should attach to.

        Returns:
            True if the debugger successfully attached to the server and
            process.
        '''
        if type(remote_pid) is int:
            remote_pid = str(remote_pid)

        err1 = dbg.SetCurrentPlatform('remote-android')
        if err1.Fail():
            return False

        self.platform_ = dbg.GetSelectedPlatform()
        if not self.platform_:
            return False

        connect_string = 'connect://localhost:%s' % Config.get_device_port()
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
        '''Append a string to the log.

        Args:
            cmd: The string to append to the log.
        '''
        if self.log_ is None:
            self.log_ = []
        self.log_.append(cmd)

    def find_in_output(self, lldb, cmd_interp, cmd, strings_to_find):
        '''Runs an lldb command and
           checks the output for a series of strings.

        Args:
            lldb: A handle to the lldb module.
            cmd_interp: The instance of the SBCommandInterpreter that runs the
                command.
            cmd: The string that is the lldb command to run.
            strings_to_find: A list of strings that should be present in lldb's
                output.

        Returns:
            True if all expected strings were found in the command interpreter's
            output, False otherwise.
        '''
        res = lldb.SBCommandReturnObject()

        self.log_append(cmd)

        cmd_interp.HandleCommand(cmd, res)
        if not res.Succeeded():
            print 'Command "%s" failed.' % cmd
            print ' The output was: "%s"' % res.GetError()
            return False

        output = res.GetOutput()
        if not output:
            print 'Command "%s" had no output.' % cmd
            return False

        self.log_append(output)

        for string in strings_to_find:
            if not string in output:
                print 'Expected to find "%s" in the output.' % string
                print ' Found: "%s"' % output
                return False

        return True

    class TestFail(Exception):
        '''An exception that is thrown when a line in a test fails,
           i.e. a lldb command does not return the expected string.'''
        pass

    def test_assert(self, cond):
        '''Check a given condition and raise the TestFail exception
           if it is False.

        Args:
            cond: The boolean condition to check.

        Raises:
            TestFail: The condition was false.
        '''
        if not cond:
            raise self.TestFail()

    def try_command(self, cmd, expected):
        '''run an lldb command and match the expected response

        Args:
            cmd: The string representing the lldb command to run.
            expected: A list of strings that should be present in lldb's
                output.

        Raises:
            TestFail: One of the expected strings were not found in the lldb
            output.
        '''
        assert self.lldb_
        assert self.ci_
        self.test_assert(self.find_in_output(self.lldb_,
                                             self.ci_,
                                             cmd,
                                             expected))
