#!/usr/bin/env python

from collection         import collection
from bundle             import bundle
from exception          import exception
from android            import android_util
from lldb_util          import lldb_util
from config             import config
from log                import log
import time

# The RenderScript testsuite
class test_suite:

    collection_  = None
    bundle_      = None
    android_     = None
    stat_total_  = 0
    stat_passes_ = 0

    # Initialisation code. Validates the environment, pushes the binaries, adds the tests.
    def pre_run(self):
        self.android_ = android_util()
        assert not self.android_ is None

        if not self.android_.validate_adb():
            raise exception("unable to locate android debug bridge")

        self.android_.adb_root()

        self.android_.validate_device()

        self.android_.clean_device()

        self.bundle_ = bundle(self.android_)
        assert not self.bundle_ is None
        self.bundle_.push_all()

        self.collection_ = collection()
        assert not self.collection_ is None
        self.collection_.discover()

        self.android_.forward_port(config.get_host_port(), config.get_device_port())

    # if a target is launched it returns its pid
    def launch_target(self, target):

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

    # Stop a binary, which involves stopping if it is an apk or killing it is a binary (ndk built).
    def kill_target(self, target):
        assert target

        process_name = None

        if self.bundle_.is_apk(target):
            process_name = self.bundle_.get_package(target)
            self.android_.stop_app(process_name)
        else:
            process_name = target
            self.android_.kill_process(target)

    # Run a given test.
    def run_test(self, test, target, target_pid):

        if target:
            log.write("  {0}:{1}: ".format(test.get_name(), target))
        else:
            log.write("  {0}: ".format(test.get_name()))

        status = "fail"

        sbdebugger = lldb_util.create_debugger()
        assert sbdebugger
        try:
            if test.run(sbdebugger, target_pid, lldb_util.get_module()):
                self.stat_passes_ += 1
                status = "pass"
        except exception as e:
            status = "error"
            log.log(e)

        # clean up after running the test
        test.post_run()
        log.log(test.get_log())

        log.writeln("{0}".format(status))

        assert sbdebugger
        lldb_util.destroy_debugger(sbdebugger)

        # give the host and device a chance to tare down
        time.sleep(2)

    # Run all tests
    def run_all(self):
        log.writeln("running:")
        lldb_util.start()

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
                                log.writeln("unable to spawn target, skipping test")
                            else:
                                output = self.android_.launch_lldbserver(config.get_device_port())
                                if output:
                                    log.log(output)
                                self.run_test(test, target, target_pid)
                                self.android_.kill_pid(target_pid)
                                self.android_.clean_device()
                    else:
                        # test has no target, i.e. requires no binary to attach to.
                        self.stat_total_ += 1
                        self.run_test(test, None, None)

                except exception as e:
                    pass

            lldb_util.stop()

        except exception as e:
            print e.get()
            return False

    # Cleanup code. Cleans up the android module.
    def tear_down(self):
        self.android_.tear_down()

    # Print a summary of pass/fail.
    def summary(self):
        perc = (self.stat_passes_ * 100) / self.stat_total_
        log.writeln("{0} of {1} passed: {2}%".format(self.stat_passes_, self.stat_total_, perc))

# Main entry into the testsuite.
def main():
    suite = test_suite()
    assert not suite is None

    try:
        suite.pre_run()
        suite.run_all()
        suite.tear_down()
        suite.summary()
    except exception as e:
        log.writeln(e)

if __name__ == "__main__":
    main()
