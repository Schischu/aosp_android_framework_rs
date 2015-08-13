#!/usr/bin/env python
'''This script will run one specific test'''
import sys
import atexit
from threading import Timer
import os

import harness
from harness import util_constants
from harness.util_functions import load_py_module
from harness.util_lldb import UtilLLDB
from harness.exception import TestSuiteException

from test_breakpoint_fileline_debug import TestBreakpointFileLineDebug
from test_breakpoint_kernel_debug_1 import TestBreakpointKernelDebug1
from test_breakpoint_kernel_debug_2 import TestBreakpointKernelDebug2
from test_language import TestLanguage
from test_language_subcmds_debug import TestLanguageSubcmdsDebug
from test_read_global import TestReadGlobal
from test_backtrace import TestBacktrace
from test_source_step import TestSourceStep
from test_read_local import TestReadLocal

# dictionary of test path, to test instance
TEST_MAP = {
    'test_breakpoint_fileline_debug.py':
    TestBreakpointFileLineDebug,
    'test_breakpoint_kernel_debug_1.py':
    TestBreakpointKernelDebug1,
    'test_breakpoint_kernel_debug_2.py':
    TestBreakpointKernelDebug2,
    'test_language.py':
    TestLanguage,
    'test_language_subcmds_debug.py':
    TestLanguageSubcmdsDebug,
    'test_read_global.py':
    TestReadGlobal,
    "test_backtrace.py":
    TestBacktrace,
    'test_source_step.py':
    TestSourceStep,
    'test_read_local.py':
    TestReadLocal
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
    state.log_.writeln('running: {0}'.format(state.name_))

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
        state.log_.writeln('test {0} failed'.format(state.name_))
        return False

    return True

def on_timeout():
    '''This is a callback function that will fire if a test takes
       longer then a threshold time to complete'''
    # pylint: disable=protected-access
    print util_constants.TIMEOUT_TOKEN
    sys.stdout.flush()
    # hard exit to force kill all threads that may block our exit
    os._exit(1)

def start_timeout():
    '''This function will start a timer that will act as a timeout
       killing this test session if a test becomes un-responsive

    Returns: The instance of the Timer class that was created
    '''
    timer = Timer(util_constants.TIMEOUT, on_timeout)
    timer.start()
    atexit.register(stop_timeout, timer)
    return timer

def stop_timeout(timer):
    '''This function will stop the timeout timer and kill its
    associated thread

    Args:
        timer: The Timer instance that is to be stopped
    '''
    if timer:
        timer.cancel()

def quit_test(num, timer):
    '''This function will safely quit the test, making sure the timeout thread
    is also killed

    Args:
        num: An integer specifying the exit status, 0 meaning
            "successful termination"
        timer: The current Timer instance
    '''
    stop_timeout(timer)
    sys.stdout.flush()
    sys.exit(num)

def execute_test(state, log):
    ''' Execute
    Args:
        state: The current State object
        log: The current Log object
    '''
    try:
        if not test_pre_run(state):
            raise TestSuiteException('test_pre_run() failed')
        if not test_run(state):
            raise TestSuiteException('test_run() failed')
        # output a generic pass token
        log.writeln(util_constants.PASS_TOKEN)
    except TestSuiteException as error:
        log.writeln(str(error))

    state.test_.post_run()

def main():
    '''Test runner entry point'''

    # re-open stdout with no buffering
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)

    # start the timeout counter
    timer = start_timeout()
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
                         pid_=None,
                         name_=args.test_name_)
                     )

        # execute the test case
        execute_test(state, log)
        # tear down the lldb instance
        UtilLLDB.destroy_debugger(lldb)
        quit_test(0, timer)

    except AssertionError:
        print 'Internal test suite error'
        quit_test(1, timer)

    except TestSuiteException as error:
        if log:
            log.write(str(error))
        else:
            print str(error)
        quit_test(2, timer)

    # use a global exception handler to be sure that we will
    # exit safely and correctly
    except:
        quit_test(3, timer)

# execution trampoline
if __name__ == '__main__':
    main()
