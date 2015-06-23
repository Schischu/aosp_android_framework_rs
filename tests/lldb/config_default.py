'''This file contains the default test suite config which will be used in
   the case a developer did not supply a custom one'''

import os

class Config(object):
    '''This Config class is used by the test suite to abstract the specifics
       of a user's local setup. This config can be overridden by specifying a
       custom config on the command line.'''

    @staticmethod
    def get_adb_path():
        '''Path to android debug bridge'''
        return 'adb'

    @staticmethod
    def get_host_port():
        '''The port to forward to lldb-server on the host'''
        return 1234

    @staticmethod
    def get_device_port():
        '''The port to forward to lldb-server on the device'''
        return 1234

    @staticmethod
    def get_lldb_server_path():
        '''Path to lldb-server on the device'''
        return '/data/lldb-server-3.7.0'

    @staticmethod
    def get_aosp_product_path():
        '''The path to the "out" folder of the AOSP'''
        return os.getenv('ANDROID_PRODUCT_OUT')

    @staticmethod
    def get_log_file_path():
        '''The path to the folder where the log file will be placed'''
        return os.getcwd()

    @staticmethod
    def get_lldb_path():
        '''The path to lldb on the host'''
        return 'lldb'
