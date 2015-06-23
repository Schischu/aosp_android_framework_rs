#!/usr/bin/python2.7
'''Main test suite execution script'''

import os
import subprocess
import sys

from tests.harness import util_constants
from tests.harness import TestSuiteException
from tests.harness import UtilAndroid
from tests.harness import UtilBundle
from tests.harness import UtilLog
from tests.harness.util_functions import load_py_module

class State(object):
    '''This class manages all objects required by the test suite'''

    def __init__(self):
        '''State constructor

        Raises:
            TestSuiteException: When unable to load config file

            AssertionError: When assertions fail
        '''

        # discover config file path
        conf_path = 'config_default.py'
        if len(sys.argv) > 1:
            conf_path = sys.argv[1]

        # create a config instance
        self.config_module_ = load_py_module(conf_path)
        if not self.config_module_:
            raise TestSuiteException('unable to load config file')
        assert self.config_module_
        self.config_ = self.config_module_.Config()
        self.config_path_ = conf_path

        # create result array
        self.results_ = dict()

        # create a log
        self.log_ = UtilLog()
        assert self.log_
        self.log_.open(self.config_.get_log_file_path(), True)

        # create an android helper object
        self.android_ = UtilAndroid(self.log_, self.config_)
        assert self.android_

        # create a test bundle
        self.bundle_ = UtilBundle(self.android_,
                                  self.config_,
                                  self.log_)
        assert self.bundle_

    def get_config(self):
        '''Return the config instance

        Returns:
            The config class, instance of Config
        '''
        return self.config_

    def get_config_path(self):
        '''Return path to config file

        Returns:
            String file path to the provided config file
        '''
        return self.config_path_

    def get_log(self):
        '''Return the test suite log instance

        Returns:
            The log class, instance of UtilLog
        '''
        assert self.log_
        return self.log_

    def get_android(self):
        '''Return the android ADB helper instance

        Returns:
            The android ADB helper, instance of UtilAndroid
        '''
        assert self.android_
        return self.android_

    def get_bundle(self):
        '''Return the test executable bundle

        Returns:
            The test exectable collection, instance of UtilBundle
        '''
        return self.bundle_

    def add_result(self, name, result):
        '''Add a test result to the collection

        Args:
            name:   String name of the test that has executed
            result: result: String result of the test, "pass", "fail", "error"
        '''
        self.results_[name] = result

def run_test(state, name):
    '''Execute a single test case

    Args:
        state: Test suite state collection, instance of State
        name: String file name of the test to execute

    Returns:
        True if the specific test was executed till it returned.
        False if an error halted test execution.

    Raises:
        AssertionError: When assertion fails
    '''
    assert type(name) is str

    state.log_.write('running {0}: '.format(name))
    params = ['python2.7',
              'tests/run_test.py',
              name,
              state.get_config_path()]
    proc = subprocess.Popen(params,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    if not proc:
        return False
    (stdout, _) = proc.communicate()
    if stdout:
        if stdout.find(util_constants.PASS_TOKEN) >= 0:
            state.add_result(name, 'pass')
            state.log_.writeln('pass')
        else:
            state.add_result(name, 'fail')
            state.log_.writeln(stdout)
    else:
        state.add_result(name, 'error')
        state.log_.writeln('error')
    return True

def suite_pre_run(state):
    '''This function is executed before the test cases are run (setup)

    Args:
        state: Test suite state collection, instance of State

    Return:
        True if the pre_run step completes without error.
        Checks made:
            Validating that adb exists and runs
            Validating that a device is attached
            We have root access to the device
            All test binaries were pushed to the device
            The port for lldb-server was forwarded correctly

    Raises:
        AssertionError: When assertions fail
    '''
    total = 0
    assert state
    try:
        android = state.get_android()
        bundle = state.get_bundle()
        log = state.get_log()
        assert android
        assert bundle
        assert log

        # validate ADB helper class
        android.validate_adb()
        android.validate_device()
        log.writeln('Located ADB')
        # elevate to root user
        android.adb_root()
        android.wait_for_device()
        # push all tests to the device
        bundle.push_all()
        log.writeln('Pushed all tests')
        # forward port for lldb-server
        config = state.get_config()
        android.forward_port(config.get_host_port(),
                             config.get_device_port())
        log.writeln('Pre run complete')

    except TestSuiteException as expt:
        print str(expt)
        return False
    return True

def suite_post_run(state):
    '''This function is executed after the test cases have run (teardown)

    Args:
        state: Test suite state collection, instance of State
    '''
    total = 0
    passes = 0
    for _, value in state.results_.iteritems():
        total += 1
        if value == 'pass':
            passes += 1
    print '{0} of {1} passed'.format(passes, total)
    if total:
        print '{0}% rate'.format((passes*100)/total)

def discover_tests(_):
    '''Discover all tests in the tests directory

    Returns:
        List of strings, test file names from the tests/ dir
    '''
    tests = []
    for item in os.listdir('tests'):
        if item.startswith('test') and item.endswith('.py'):
            tests.append(item)
    return tests

def deduce_python_path(state):
    '''Try to deduce PYTHONPATH via the LLDB binary given in the config

    Args:
        state: Test suite state collection, instance of State

    Returns:
        True if PYTHONPATH has been updated, False otherwise

    Raises:
        TestSuiteException: If lldb path in config is incorrect
        AssertionError: If an assertion fails
    '''
    config = state.get_config()
    assert config
    lldb_path = config.get_lldb_path()
    if not lldb_path:
        # lldb may not be provided in preference of a manual $PYTHONPATH
        return False
    if not os.path.exists(lldb_path):
        raise TestSuiteException('Invalid lldb path specified in config')
    params = [lldb_path, '-P']
    proc = subprocess.Popen(params, stdout=subprocess.PIPE)
    stdout = proc.communicate()[0]
    if stdout:
        stdout = stdout.replace('\n', '')
        os.environ['PYTHONPATH'] = stdout
        return True
    return False

def main():
    '''Test suite entry point'''

    try:
        # parse the command line
        state = State()
        assert state
        if not state.get_config():
            raise TestSuiteException('Unable to load a config file')

        # if we can, set PYTHONPATH for lldb bindings
        if not deduce_python_path(state):
            print 'Unable to deduce PYTHONPATH'
        # pre run step
        if not suite_pre_run(state):
            raise TestSuiteException('Test suite pre-run step failed')
        # discover all tests and execute them
        tests = discover_tests(state)
        print 'Found {0} tests'.format(len(tests))
        for item in tests:
            if not run_test(state, item):
                raise TestSuiteException('Error while trying to execute test')
        # post run step
        suite_post_run(state)

        # success
        quit(0)

    except AssertionError:
        print 'Internal test suite error'
        quit(1)

    except TestSuiteException as error:
        print '{0}'.format(str(error))
        quit(2)

# execution trampoline
if __name__ == '__main__':
    main()
