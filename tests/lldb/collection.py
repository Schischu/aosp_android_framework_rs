'''Module that contains the class Collection,
   which represents a set of tests.'''

import tests

class Collection(object):
    '''Represents a collection of all tests to run.'''

    tests_ = []

    def __init__(self):
        pass

    def discover(self):
        '''Adds all tests to a list.'''
        self.tests_.append(
            tests.TestCmdLanguage())
        self.tests_.append(
            tests.language_subcmds.TestCmdLanguageSubcmdsDebug())
        self.tests_.append(
            tests.breakpoint_kernel.TestCmdBreakpointKernelDebug1())
        self.tests_.append(
            tests.breakpoint_kernel.TestCmdBreakpointKernelDebug2())
        self.tests_.append(
            tests.breakpoint_fileline.TestCmdBreakpointFileLineDebug())

    def get_tests(self):
        '''Get the list of tests.'''
        return self.tests_
