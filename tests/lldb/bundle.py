'''Module that contains the class Bundle,
   representing a collection of RS binaries.'''

from config import Config
from exception import TestSuiteException
from log import LOG

class Bundle(object):
    '''Represents the collection of RS binaries
       that are debugged by the tests.'''

    android_ = None # Link to the android module

    # Map of binary name to package name of all Java apps debugged
    tests_apk_ = {
                  'JavaInfiniteLoop': 'com.android.rs.infiniteLoop',
                  'JavaDebugWaitAttach': 'com.android.rs.waitattachdebug'
                  }

    # List of binary names of all NDK apps debugged
    tests_ndk_ = {}
#                  'CppNoDebugWaitAttach',
#                  'CppDebugWaitAttach',
#                  'CppInfiniteLoop'

    def __init__(self, android):
        self.android_ = android

    def is_apk(self, name):
        '''Checks if a binary of a given name is an apk, i.e. if it is in the
           dictionary of apks.

        Args:
            name: The string that is the name of the binary to check.

        Returns:
            True if the binary is an apk, False if it is not.

        Raises:
            TestSuiteException: The string does not match any item in the list
            of APK or NDK binaries.
        '''
        if name in self.tests_apk_:
            return True
        if not name in self.tests_ndk_:
            raise TestSuiteException('test not apk or ndk')
        return False

    def push_all(self):
        '''Push all apk and ndk binaries to the device.

        Raises:
            TestSuiteException: One or more apks could not be installed.
        '''
        self.push_all_apk()
        self.push_all_ndk()

    def push_all_apk(self):
        '''Push all apk files to the device,
           which involves uninstalling any old installations and installing.

        Raises:
            TestSuiteException: An apk could not be installed.
        '''
        product_folder = Config.get_aosp_product_path()
        app_folder = product_folder + '/data/app'

        for app, package in self.tests_apk_.iteritems():

            LOG.writeln('pushing {0}'.format(app))

            self.android_.stop_app(package)

            self.android_.adb('uninstall ' + package)
            # Ignore the output of uninstall.
            # The app may not have been installed in the first place. That's ok.

            flags = ''
            abi = self.android_.get_abi()
            if 'arm64-v8a' in abi:
                flags = '--abi armeabi-v7a'

            cmd = 'install {0} {1}/{2}/{2}.apk'.format(flags, app_folder, app)
            output = self.android_.adb(cmd)
            if (not 'Success' in output) or ("can't find" in output):
                raise TestSuiteException('unable to install app ' + app)

        return

    def push_all_ndk(self):
        '''Push all ndk binaries to the device.

        Raises:
            TestSuiteException: A binary could not be pushed to the device.
        '''
        product_folder = Config.get_aosp_product_path()
        app_folder = product_folder + '/system/bin'

        for app in self.tests_ndk_.iteritems():

            LOG.writeln('pushing {0}'.format(app))

            self.android_.kill_process(app)

            cmd = 'push %s/%s /data' % (app_folder, app)
            output = self.android_.adb(cmd)
            if not 'KB/s' in output or 'failed to copy' in output:
                raise TestSuiteException('unable to push binary ' + app)

            # be sure to set the execute bit for NDK binaries
            self.android_.adb('shell chmod 777 /data/{0}'.format(app))

    def get_package(self, app_name):
        '''From a given apk name get the name of its package.

        Args:
            app_name: The string that is the name of the apk.

        Returns:
            A string representing the name of the package of the app.

        Raises:
            TestSuiteException: The app name is not in the list of apks.
        '''
        if not app_name in self.tests_apk_:
            msg = ('unknown app %s. (Do you need to add an '
                  'entry to bundle.py :: test_apps_?)' % app_name)
            raise TestSuiteException(msg)

        return self.tests_apk_[app_name]
