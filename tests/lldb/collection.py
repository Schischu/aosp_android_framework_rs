import tests

# Represents a collection of all tests to run.
class collection:

    tests_ = []

    # Adds all tests to a list.
    def discover(self):
        self.tests_.append(tests.test_cmd_language())
        self.tests_.append(tests.language_subcmds.test_cmd_language_subcmds_debug())
        self.tests_.append(tests.breakpoint_kernel.test_cmd_breakpoint_kernel_debug_1())
        self.tests_.append(tests.breakpoint_kernel.test_cmd_breakpoint_kernel_debug_2())
        self.tests_.append(tests.breakpoint_fileline.test_cmd_breakpoint_fileline_debug())

    # Get the list of tests.
    def get_tests(self):
        return self.tests_
