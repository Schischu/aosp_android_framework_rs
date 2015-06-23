'''Module that contains the Config class, which contains user specific data.'''

class Config(object):
    '''Configuration file that contains some paths and constants
       that can be modified by a user.
       Modify this file to contain your local values!'''

    @staticmethod
    def get_adb_path():
        '''path to android debug bridge

        Returns:
            A string representing the path.
        '''
        return 'adb'

    @staticmethod
    def get_host_port():
        '''the port to forward to lldb-server on the host

        Returns:
            An integer representing the port.
        '''
        return 1234

    @staticmethod
    def get_device_port():
        '''the port to forward to lldb-server on the device

        Returns:
            An integer representing the port.
        '''
        return 1234

    @staticmethod
    def get_lldb_server_path():
        '''path of lldb-server on the device

        Returns:
            An string representing the path.
        '''
        assert False and ('In config.py set get_lldb_server_path()'
                          ' to your lldb-server on the device.')
        # example
        return '/data/lldb-server-3.7.0'

    @staticmethod
    def get_aosp_product_path():
        '''the path to the "out" folder of the AOSP

        Returns:
            An string representing the path.
        '''
        assert False and ('In config.py set get_aosp_product_path()'
                          ' to your AOSP product path.')
        # example
        return '/home/aidan/rs/aosp/out/target/product/shamu'

    @staticmethod
    def get_log_file_path():
        '''the path to the folder where the log file will be placed

        Returns:
            An string representing the path.
        '''
        assert False and ('In config.py set get_log_file_path()'
                          ' to a valid location for the log file.')
        # example
        return '/home/aidan'
