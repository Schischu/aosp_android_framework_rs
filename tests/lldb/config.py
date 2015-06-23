# Configuration file that contains some paths and constants that can be modified by a user.
# Modify this file to contain your local values!
class config:

    # path to android debug bridge
    @staticmethod
    def get_adb_path():
        return "adb"

    # path to port forward lldb-server on the host
    @staticmethod
    def get_host_port():
        return 1234

    # path for lldb-server to host on device
    @staticmethod
    def get_device_port():
        return 1234

    # path of lldb-server on the device
    @staticmethod
    def get_lldb_server_path():
        assert False and "In config.py set get_lldb_server_path() to your lldb-server on the device."
        # example
        return "/data/lldb-server-3.7.0"

    # the path to the "out" folder of the AOSP
    @staticmethod
    def get_AOSP_product_path():
        assert False and "In config.py set get_AOSP_product_path() to your AOSP product path."
        # example
        return "/home/aidan/rs/aosp/out/target/product/shamu"

    # the path to the folder where the log file will be placed
    @staticmethod
    def get_log_file_path():
        assert False and "In config.py set get_log_file_path() to a valid location for the log file."
        # example
        return "/home/aidan"
