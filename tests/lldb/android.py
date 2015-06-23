import subprocess
import time
from exception import exception
from config import config
from log import log

# Provides some utility methods that interface with Android using adb.
class android_util:

    m_path_adb        = ""
    m_path_gdbserver  = ""
    m_path_lldbserver = ""

    # Check that a string is valid and not empty
    @staticmethod
    def _validate_string(string):
        assert type(string) is str
        assert len(string) > 0

    # ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
    # constructor
    #
    def __init__(self):
        self.m_path_adb = config.get_adb_path()
        self.m_path_lldbserver = config.get_lldb_server_path()
        return

    # Destructor. Closes the log file.
    def tear_down(self):
        pass

    # run an adb command (async optional)
    def adb(self, args, async=False):
        try:
            # form the command
            if self.m_path_adb:
                cmd = self.m_path_adb + ' ' + args
            else:
                cmd = "adb {0}".format(args)
            log.log(cmd)

            # spawn async thread
            if async:
                subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

            # spawn blocking
            else:
                # execute the command
                proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
                (stdout, stderr) = proc.communicate(None)
                log.log(stdout)
                log.log(stderr)
                return stdout + stderr

        except Exception as e:
            raise exception('unable to execute adb: {0}'.format(str(e)))
        return

    # return the device abi
    def get_abi(self):
        return self.shell("getprop ro.product.cpu.abi")

    # run something via the adb shell
    def shell(self, cmd, async=False):
        return self.adb("shell '{0}'".format(cmd), async)

    # find an app pid
    def find_app_pid(self, process_name):
        self._validate_string(process_name)
        stdout = self.shell("ps | grep " + process_name)

        try:
            # reduce white space to single space
            stdout = stdout.replace('\t', ' ')
            for i in range(1, 5):
                stdout = stdout.replace('  ', ' ')

            items = stdout.split(' ')
            if len(items) <= 1:
                log.log('unable to find pid of: {0}'.format(process_name))
                return

            pid = int(items[1])
            log.log('app pid found: {0}'.format(items[1]))
            return pid

        except Exception as e:
            return None

    # Set adb to be in root mode
    def adb_root(self):
        out = self.adb('root')

    # Validate adb that it can be run
    def validate_adb(self):
        out = self.adb('version')
        if out and (out.find('Android') >= 0) and (out.find('version') >= 0):
            log.log("adb found: {0}".format(out))
            return True
        return False

    # Validate that there is at least one device
    def validate_device(self):
        out = self.adb('devices')
        if not "List of devices attached" in out:
            raise exception("Unable to list devices")

        found_device = False

        lines = out.split('\n')
        for line in lines[1:]:
            if "\tdevice" in line:
                if found_device:
                    raise exception("adb found multiple devices. This is currently not supported by the test suite."
                                    "Please disconnect all but one.")

                found_device = True

        if not found_device:
            raise exception("adb is unable to find a connected device/emulator to test.")

        return

    # Kill a process identified by its pid by issuing a "kill" command
    def kill_pid(self, pid):
        self.shell("kill " + str(pid))

    # Terminate an app by calling am force-stop
    def stop_app(self, package_name):
        self._validate_string(package_name)
        self.shell("am force-stop " + package_name)

    # Kill a process identified by its name (package name in case of apk) by issuing a "kill" command
    def kill_process(self, name):
        pid = self.find_app_pid(name)
        if pid:
            self.kill_pid(pid)
            return True
        return False

    def kill_all_processes(self, name):

        # try 5 times to kill this process
        for i in range(1, 5):
            if not self.kill_process(name):
                return
        # stalled process must reboot
        self.reboot_device()

    # Clean the device state, killing all gdbserver and lldb-server instances.
    def clean_device(self):
        self.kill_all_processes("gdbserver")
        self.kill_all_processes("lldb-server")

    # Launch a binary (compiled with the NDK)
    def launch_elf(self, binary_name, args, env):
        cmd = ''
        if env:
            for x in env:
                cmd += 'export ' + x + '="' + env[x] + '";'

        cmd += 'exec /data/' + binary_name

        if type(args) is str:
            cmd += args

        if type(args) is list:
            for x in args:
                cmd += x + ' '

        stdout = self.shell("\"" + cmd + "\"", True)
        if type(stdout) is str:
            log.log(stdout)

        return

    # reboot the remote device
    def reboot_device(self):
        self.adb("reboot")
        self.adb('wait-for-device')
        time.sleep(1)
        self.adb("root")
        time.sleep(1)
        self.adb("remount")
        time.sleep(1)

    # launch a render script app
    def launch_app(self, name, activity):
        assert name and activity

        cmd = 'am start -S -n ' + name + '/' + name + '.' + activity
        stdout = self.shell(cmd)

        if type(stdout) is str:
            log.log(stdout)
        return None

    # launch lldb server and attach to target app
    def launch_lldbserver(self, port):
        channels = ["gdb-remote all",
                    "lldb breakpoints",
                    "lldb dyld",
                    "lldb process",
                    "lldb platform"]

        opts = "-l data/lldblog.txt "
        chans = "-c "
        for x in channels:
            chans += "{0};".format(x)
        opts += chans

        cmd = "export LLDB_DEBUGSERVER_PATH='{0}';{0} p --listen {1}".format(self.m_path_lldbserver, port)

        self.shell(cmd, True)
        time.sleep(5)
        return

    # use adb to forward a device port onto the local machine
    def forward_port(self, local, remote):
        # ask adb to forward this port
        cmd = 'forward tcp:' + str(local) + ' tcp:' + str(remote)
        self.adb(cmd)
        return
