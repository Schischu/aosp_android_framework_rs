'''Module that contains the class Collection,
   which represents a set of tests.'''

import tests

class Collection(object):
    '''Represents a collection of all tests to run.'''

    # List of all python tests.
    # Will be initalised in the discover method.
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
        '''Get the list of tests.

        Returns:
            An list of objects with TestBase base class that is the list of all
                tests to run.
        '''
        return self.tests_
