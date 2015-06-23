'''This script will run one specific test'''
import sys
import atexit

import harness
from harness import util_constants
from harness.util_functions import load_py_module
from harness.util_lldb import UtilLLDB
from harness.exception import TestSuiteException

from test_cmd_breakpoint_fileline_debug import TestCmdBreakpointFileLineDebug
from test_cmd_breakpoint_kernel_debug_1 import TestCmdBreakpointKernelDebug1
from test_cmd_breakpoint_kernel_debug_2 import TestCmdBreakpointKernelDebug2
from test_cmd_language import TestCmdLanguage
from test_cmd_language_subcmds_debug import TestCmdLanguageSubcmdsDebug

# dictionary of test path, to test instance
TEST_MAP = {
    'test_cmd_breakpoint_fileline_debug.py':
    TestCmdBreakpointFileLineDebug,
    'test_cmd_breakpoint_kernel_debug_1.py':
    TestCmdBreakpointKernelDebug1,
    'test_cmd_breakpoint_kernel_debug_2.py':
    TestCmdBreakpointKernelDebug2,
    'test_cmd_language.py':
    TestCmdLanguage,
    'test_cmd_language_subcmds_debug.py':
    TestCmdLanguageSubcmdsDebug,
    }

class CmdArgs(object):
    '''This class is a simple command line wrapper'''

    def __init__(self):
        '''Constructor

        Raises:
            TestSuiteException: When 2 commandline arguments are not given
        '''
        if len(sys.argv) < 3:
            raise TestSuiteException(
                'Invalid number of arguments')
        self.test_name_ = sys.argv[1]
        self.config_path_ = sys.argv[2]

    def get_test_name(self):
        '''Return name of the test being run

        Returns:
            String name of the test to be executed
        '''
        return self.test_name_

    def get_config_path(self):
        '''Return path to the config file to use

        Returns:
            String path to the config file to use
        '''
        return self.config_path_


def test_pre_run(state):
    '''This function is called before a test is executed (setup)

    Args:
        state: Test suite state collection, instance of State.

    Returns:
        True if the pre_run step completed with out error.
        Currently the pre-run will launch the target test binary
        on the device and attach an lldb-server to it in platform
        mode.

    Raises:
        AssertionError: If an assertion fails.
    '''
    assert state.test_
    assert state.bundle_

    assert state.log_
    state.log_.writeln('running: {0}'.format(state.test_.get_name()))

    target_name = state.test_.get_bundle_target()
    if target_name:
        state.pid_ = state.bundle_.launch(target_name)
        if not state.pid_:
            state.log_.writeln('unable to get pid of target')
            return False

    state.android_.launch_lldb_platform(state.config_.get_device_port())
    return True

def test_run(state):
    '''Execute a single test case

    Args:
        state: test suite state collection, instance of State

    Returns:
        True if the test case ran successfully and passed.
        False if the test case failed or suffered an error.

    Raises:
        AssertionError: If an assertion fails.
    '''
    assert state.lldb_
    assert state.config_
    assert state.lldb_module_
    assert state.test_

    if not state.test_.run(state.lldb_, state.pid_, state.lldb_module_):
        assert state.log_
        state.log_.log(state.test_.get_log())
        state.log_.writeln('test {0} failed'.format(state.test_.get_name()))
        return False

    return True

def main():
    '''Test runner entry point'''

    log = None

    try:
        # startup lldb and register teardown handler
        atexit.register(UtilLLDB.stop)
        UtilLLDB.start()

        # parse the command line
        args = CmdArgs()
        assert args

        # load a config file from disk
        config = load_py_module(args.config_path_)
        if config is None:
            raise TestSuiteException('Unable to load config file')
        conf_inst = config.Config()
        assert conf_inst

        # create utility classes
        log = harness.UtilLog()
        assert log
        log_path = conf_inst.get_log_file_path()
        log.open(log_path, False)
        android = harness.UtilAndroid(log, conf_inst)
        bundle = harness.UtilBundle(android, conf_inst, log)

        # check the requested test exists
        if args.test_name_ not in TEST_MAP:
            raise TestSuiteException(
                'unable to find test: {0}'.format(args.test_name_))
        test_class = TEST_MAP[args.test_name_]
        assert test_class
        test_inst = test_class(conf_inst)
        assert test_inst

        # create an lldb instance
        lldb = UtilLLDB.create_debugger()

        # create state object to encapsulate instances
        state = type('State',
                     (object,),
                     dict(
                         log_=log,
                         android_=android,
                         bundle_=bundle,
                         lldb_=lldb,
                         lldb_module_=UtilLLDB.get_module(),
                         test_=test_inst,
                         config_=conf_inst,
                         pid_=None)
                     )

        # execute the test case
        try:
            if not test_pre_run(state):
                raise TestSuiteException('test_pre_run() failed')
            if not test_run(state):
                raise TestSuiteException('test_run() failed')
            # output a generic pass token
            log.writeln(util_constants.PASS_TOKEN)
        except TestSuiteException as error:
            log.writeln(str(error))

        # tear down the lldb instance
        UtilLLDB.destroy_debugger(lldb)
        quit(0)

    except AssertionError:
        print 'Internal test suite error'
        quit(1)

    except TestSuiteException as error:
        if log:
            log.write(str(error))
        else:
            print str(error)
        quit(2)

# execution trampoline
if __name__ == '__main__':
    main()
