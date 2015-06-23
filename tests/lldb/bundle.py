'''Module that contains the class Bundle,
   representing a collection of RS binaries.'''

from config import Config
from exception import TestSuiteException
from log import LOG

class Bundle(object):
    '''Represents the collection of RS binaries
       that are debugged by the tests.'''

    android_ = None

    tests_apk_ = {
                  "JavaInfiniteLoop": "com.android.rs.infiniteLoop",
                  "JavaDebugWaitAttach": "com.android.rs.waitattachdebug"
                  }

    tests_ndk_ = {}
#                  "CppNoDebugWaitAttach": "",
#                  "CppDebugWaitAttach": "",
#                  "CppInfiniteLoop": ""

    def __init__(self, android):
        self.android_ = android

    def is_apk(self, name):
        '''Checks if a binary of a given name is an apk, i.e.'''
        if name in self.tests_apk_:
            return True
        if not name in self.tests_ndk_:
            raise TestSuiteException("test not apk or ndk")
        return False

    def push_all(self):
        '''Push all apk and ndk binaries to the device.'''
        self.push_all_apk()
        self.push_all_ndk()

    def push_all_apk(self):
        '''Push all apk files to the device,
           which involves uninstalling any old installations and installing.'''
        product_folder = Config.get_aosp_product_path()
        app_folder = product_folder + "/data/app"

        for app, package in self.tests_apk_.iteritems():

            LOG.writeln("pushing {0}".format(app))

            self.android_.stop_app(package)

            self.android_.adb("uninstall " + package)
            # Ignore the output of uninstall.
            # The app may not have been installed in the first place. That's ok.

            flags = ''
            abi = self.android_.get_abi()
            if "arm64-v8a" in abi:
                flags = '--abi armeabi-v7a'

            cmd = "install {0} {1}/{2}/{2}.apk".format(flags, app_folder, app)
            output = self.android_.adb(cmd)
            if (not "Success" in output) or ("can't find" in output):
                raise TestSuiteException("unable to install app " + app)

        return

    def push_all_ndk(self):
        '''Push all ndk files to the device.'''
        product_folder = Config.get_aosp_product_path()
        app_folder = product_folder + "/system/bin"

        for app in self.tests_ndk_.iteritems():

            LOG.writeln("pushing {0}".format(app))

            self.android_.kill_process(app)

            cmd = "push " + app_folder + "/" + app + " /data"
            output = self.android_.adb(cmd)
            if not "KB/s" in output or "failed to copy" in output:
                raise TestSuiteException("unable to push binary " + app)

            # be sure to set the execute bit for NDK binaries
            self.android_.adb("shell chmod 777 /data/{0}".format(app))

    def get_package(self, app_name):
        '''From a given apk name get the name of its package.'''
        if not app_name in self.tests_apk_:
            msg = "unknown app " + app_name + \
                  ". (Do you need to add an entry to bundle.py :: test_apps_?)"
            raise TestSuiteException(msg)

        return self.tests_apk_[app_name]
