from config import config
from exception import exception
from log import log

# Represents the collection of RS binaries that are debugged by the tests.
class bundle:

    android_ = None

    tests_apk_ = {
                  "JavaInfiniteLoop": "com.android.rs.infiniteLoop",
                  "JavaDebugWaitAttach": "com.android.rs.waitattachdebug"
                  }

    tests_ndk_ = {}
#                  "CppNoDebugWaitAttach": "",
#                  "CppDebugWaitAttach": "",
#                  "CppInfiniteLoop": ""

    # Constructor
    def __init__(self, android):
        self.android_ = android

    #  Checks if a binary of a given name is an apk, i.e.
    def is_apk(self, name):
        if name in self.tests_apk_:
            return True
        if not name in self.tests_ndk_:
            raise exception("test not apk or ndk")
        return False

    # Push all apk and ndk binaries to the device.
    def push_all(self):
        self.push_all_apk()
        self.push_all_ndk()

    # Push all apk files to the device, which involves uninstalling any old installations and installing.
    def push_all_apk(self):
        product_folder = config.get_AOSP_product_path()
        app_folder = product_folder + "/data/app"

        for app, package in self.tests_apk_.iteritems():

            log.writeln("pushing {0}".format(app))

            self.android_.stop_app(package)

            self.android_.adb("uninstall " + package)
            # Ignore the output of uninstall. The app may not have been installed in the first place. That's ok.

            flags = ''
            abi = self.android_.get_abi()
            if "arm64-v8a" in abi:
                flags = '--abi armeabi-v7a'

            output = self.android_.adb("install {0} {1}/{2}/{2}.apk".format(flags, app_folder, app))
            if (not ("Success" in output)) or ("can't find" in output):
                raise exception("unable to install app " + app)

        return

    # Push all ndk files to the device.
    def push_all_ndk(self):
        product_folder = config.get_AOSP_product_path()
        app_folder = product_folder + "/system/bin"

        for app, package in self.tests_ndk_.iteritems():

            log.writeln("pushing {0}".format(app))

            self.android_.kill_process(app)

            output = self.android_.adb("push " + app_folder + "/" + app + " /data")
            if not "KB/s" in output or "failed to copy" in output:
                raise exception("unable to push binary " + app)

            # be sure to set the execute bit for NDK binaries
            self.android_.adb("shell chmod 777 /data/{0}".format(app))

    # From a given apk name get the name of its package.
    def get_package(self, app_name):
        if not app_name in self.tests_apk_:
            raise exception("unknown app " + app_name + ". (Do you need to add an entry to bundle.py :: test_apps_?)")

        return self.tests_apk_[app_name]
