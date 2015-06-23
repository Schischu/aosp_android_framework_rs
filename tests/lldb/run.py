#!/usr/bin/env python

'''The entry point to the RenderScript testsuite'''

from collection         import Collection
from bundle             import Bundle
from exception          import TestSuiteException
from android_util       import AndroidUtil
from lldb_util          import LLDBUtil
from config             import Config
from log                import LOG
import time

class TestSuite(object):
    '''The RenderScript testsuite'''

    collection_ = None
    bundle_ = None
    android_ = None
    stat_total_ = 0
    stat_passes_ = 0

    # Empty constructor
    def __init__(self):
        pass

    def pre_run(self):
        '''Initialisation code. Validates the environment,
        pushes the binaries, adds the tests.'''
        self.android_ = AndroidUtil()
        assert not self.android_ is None

        if not self.android_.validate_adb():
            raise TestSuiteException("unable to locate android debug bridge")

        self.android_.adb_root()

        self.android_.validate_device()

        self.android_.clean_device()

        self.bundle_ = Bundle(self.android_)
        assert not self.bundle_ is None
        self.bundle_.push_all()

        self.collection_ = Collection()
        assert not self.collection_ is None
        self.collection_.discover()

        self.android_.forward_port(Config.get_host_port(),
                                   Config.get_device_port())

    def launch_target(self, target):
        '''if a target is launched it returns its pid'''

        if not target:
            return None

        process_name = None

        if self.bundle_.is_apk(target):
            process_name = self.bundle_.get_package(target)
            self.android_.launch_app(process_name, "MainActivity")
        else:
            process_name = target
            self.android_.kill_all_processes(process_name)
            self.android_.launch_elf(target, None, None)

        # Give the app time to crash if it needs to
        time.sleep(2)

        return self.android_.find_app_pid(process_name)

    def kill_target(self, target):
        ''' Stop a binary, which involves stopping if it is an apk or
            killing it is a binary (ndk built).'''
        assert target

        process_name = None

        if self.bundle_.is_apk(target):
            process_name = self.bundle_.get_package(target)
            self.android_.stop_app(process_name)
        else:
            process_name = target
            self.android_.kill_process(target)

    def run_test(self, test, target, target_pid):
        '''Run a given test.'''
        if target:
            LOG.write("  {0}:{1}: ".format(test.get_name(), target))
        else:
            LOG.write("  {0}: ".format(test.get_name()))

        status = "fail"

        sbdebugger = LLDBUtil.create_debugger()
        assert sbdebugger
        try:
            if test.run(sbdebugger, target_pid, LLDBUtil.get_module()):
                self.stat_passes_ += 1
                status = "pass"
        except TestSuiteException as ex:
            status = "error"
            LOG.log(ex)

        # clean up after running the test
        test.post_run()
        LOG.log(test.get_log())

        LOG.writeln("{0}".format(status))

        assert sbdebugger
        LLDBUtil.destroy_debugger(sbdebugger)

        # give the host and device a chance to tare down
        time.sleep(2)

    def run_all(self):
        '''Run all tests'''
        LOG.writeln("running:")
        LLDBUtil.start()

        try:
            assert self.collection_
            for test in self.collection_.get_tests():

                try:
                    # if this test specifies a target we need to launch it
                    targets = test.get_bundle_targets()

                    if targets:
                        for target in targets:

                            self.stat_total_ += 1

                            target_pid = self.launch_target(target)

                            if target and not target_pid:
                                LOG.writeln(
                                    "unable to spawn target, skipping test")
                            else:
                                output = self.android_.launch_lldbserver(
                                    Config.get_device_port())
                                if output:
                                    LOG.log(output)
                                self.run_test(test, target, target_pid)
                                self.android_.kill_pid(target_pid)
                                self.android_.clean_device()
                    else:
                        # test has no target,
                        # i.e. requires no binary to attach to.
                        self.stat_total_ += 1
                        self.run_test(test, None, None)

                except TestSuiteException as ex:
                    pass

            LLDBUtil.stop()

        except TestSuiteException as ex:
            print ex.get()
            return False

    def summary(self):
        '''Print a summary of pass/fail.'''
        perc = (self.stat_passes_ * 100) / self.stat_total_
        LOG.writeln("{0} of {1} passed: {2}%".
                    format(self.stat_passes_, self.stat_total_, perc))

def main():
    '''Main entry into the testsuite.'''
    suite = TestSuite()
    assert not suite is None

    try:
        suite.pre_run()
        suite.run_all()
        suite.summary()
    except TestSuiteException as ex:
        LOG.writeln(ex)

if __name__ == "__main__":
    main()
