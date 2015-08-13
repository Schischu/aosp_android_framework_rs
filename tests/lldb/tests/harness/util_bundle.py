'''Module that contains the class UtilBundle,
   representing a collection of RS binaries.'''

import time

from exception import TestSuiteException

class UtilBundle(object):
    '''Represents the collection of RS binaries
       that are debugged by the tests.'''

    android_ = None # Link to the android module
    # Map of binary name to package name of all Java apps debugged
    tests_apk_ = {
                  'JavaInfiniteLoop': 'com.android.rs.infiniteLoop',
                  'JavaDebugWaitAttach': 'com.android.rs.waitattachdebug',
                  'BranchingFunCalls': 'com.android.rs.branchingfuncalls',
                  'GlobalScalarVariables': 'com.android.rs.globalscalarvariables',
                  'JavaFunction': 'com.android.rs.javafunction',
                  'KernelVariables': 'com.android.rs.kernelvariables'
                  }

    tests_ndk_ = {}

    missing_path_msg_ = 'No product path has been provided, if using the '\
                        'default environmental variable check it is set '\
                        'by running lunch. Alternatively hardcode '\
                        'your own path in the config file.'

    def __init__(self, android, config, log):
        assert android
        assert config
        assert log
        self.android_ = android
        self.config_ = config
        self.log_ = log

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
        product_folder = self.config_.get_aosp_product_path()
        if not product_folder:
            raise TestSuiteException(self.missing_path_msg_)

        app_folder = product_folder + '/data/app'

        for app, package in self.tests_apk_.iteritems():

            self.log_.writeln('pushing {0}'.format(app))

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
        product_folder = self.config_.get_aosp_product_path()
        if not product_folder:
            raise TestSuiteException(self.missing_path_msg_)

        app_folder = product_folder + '/system/bin'

        for app in self.tests_ndk_.iteritems():

            self.log_.writeln('pushing {0}'.format(app))

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

    def launch(self, app_name):
        '''Launch an apk/ndk app on a remote device

        Args:
            app_name: The string that is the name of the APK or NDK
                      executable.

        Returns:
            The Process ID of the launched executable, otherwise None
        '''

        if app_name in self.tests_apk_:
            package = self.tests_apk_[app_name]

            self.android_.kill_process(package)

            self.android_.launch_app(package, 'MainActivity')
            # Give the app time to crash if it needs to
            time.sleep(2)
            return self.android_.find_app_pid(package)

        if app_name in self.tests_ndk_:
            #TODO(Aidan): Launch NDK binary
            pass

        assert False, 'Failed to launch test executable'
        return None
