'''Module that contains the class AndroidUtil,
   providing utility method to interface with Android.'''

import subprocess
import time
from exception import TestSuiteException
from config import Config
from log import LOG

class AndroidUtil(object):
    '''Provides some utility methods that interface with Android using adb.'''

    m_path_adb = ""
    m_path_gdbserver = ""
    m_path_lldbserver = ""

    @staticmethod
    def _validate_string(string):
        '''Check that a string is valid and not empty'''
        assert type(string) is str
        assert len(string) > 0

    def __init__(self):
        self.m_path_adb = Config.get_adb_path()
        self.m_path_lldbserver = Config.get_lldb_server_path()
        return

    def adb(self, args, async=False):
        '''run an adb command (async optional)'''
        try:
            # form the command
            if self.m_path_adb:
                cmd = self.m_path_adb + ' ' + args
            else:
                cmd = "adb {0}".format(args)
            LOG.log(cmd)

            # spawn async thread
            if async:
                subprocess.Popen(cmd,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE,
                                 shell=True)

            # spawn blocking
            else:
                # execute the command
                proc = subprocess.Popen(cmd,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE,
                                        shell=True)
                (stdout, stderr) = proc.communicate(None)
                LOG.log(stdout)
                LOG.log(stderr)
                return stdout + stderr

        except TestSuiteException as ex:
            msg = 'unable to execute adb: {0}'.format(str(ex))
            raise TestSuiteException(msg)
        return

    def get_abi(self):
        '''return the device abi'''
        return self.shell("getprop ro.product.cpu.abi")

    def shell(self, cmd, async=False):
        '''run something via the adb shell'''
        return self.adb("shell '{0}'".format(cmd), async)

    def find_app_pid(self, process_name):
        '''find an app pid'''
        self._validate_string(process_name)
        stdout = self.shell("ps | grep " + process_name)

        # reduce white space to single space
        stdout = stdout.replace('\t', ' ')
        for _ in range(1, 5):
            stdout = stdout.replace('  ', ' ')

        items = stdout.split(' ')
        if len(items) <= 1:
            LOG.log('unable to find pid of: {0}'.format(process_name))
            return

        try:
            pid = int(items[1])
            LOG.log('app pid found: {0}'.format(items[1]))
            return pid
        except ValueError as ex:
            return None

    def adb_root(self):
        '''Set adb to be in root mode'''
        self.adb('root')

    def validate_adb(self):
        '''Validate adb that it can be run'''
        out = self.adb('version')
        if out and (out.find('Android') >= 0) and (out.find('version') >= 0):
            LOG.log("adb found: {0}".format(out))
            return True
        return False

    def validate_device(self):
        '''Validate that there is at least one device'''
        out = self.adb('devices')
        if not "List of devices attached" in out:
            raise TestSuiteException("Unable to list devices")

        found_device = False

        lines = out.split('\n')
        for line in lines[1:]:
            if "\tdevice" in line:
                if found_device:
                    raise TestSuiteException(
                        "adb found multiple devices. "
                        "This is currently not supported by the test suite. "
                        "Please disconnect all but one.")

                found_device = True

        if not found_device:
            raise TestSuiteException("adb is unable to find a connected "
                                     "device/emulator to test.")

        return

    def kill_pid(self, pid):
        '''Kill a process identified by its pid by issuing a "kill" command'''
        self.shell("kill " + str(pid))

    def stop_app(self, package_name):
        '''Terminate an app by calling am force-stop'''
        self._validate_string(package_name)
        self.shell("am force-stop " + package_name)

    def kill_process(self, name):
        '''Kill a process identified by its name
           (package name in case of apk) by issuing a "kill" command'''
        pid = self.find_app_pid(name)
        if pid:
            self.kill_pid(pid)
            return True
        return False

    def kill_all_processes(self, name):
        '''Repeatedly try to call "kill" on a process to ensure it is gone.
           If the process is still there after 5 attempts reboot the device.'''

        # try 5 times to kill this process
        for _ in range(1, 5):
            if not self.kill_process(name):
                return
        # stalled process must reboot
        self.reboot_device()

    def clean_device(self):
        '''Clean the device state,
           killing all gdbserver and lldb-server instances.'''
        self.kill_all_processes("gdbserver")
        self.kill_all_processes("lldb-server")

    def launch_elf(self, binary_name, args, env):
        '''Launch a binary (compiled with the NDK)'''
        cmd = ''
        if env:
            for key, value in env:
                cmd += 'export ' + key + '="' + value + '";'

        cmd += 'exec /data/' + binary_name

        if type(args) is str:
            cmd += args

        if type(args) is list:
            for arg in args:
                cmd += arg + ' '

        stdout = self.shell("\"" + cmd + "\"", True)
        if type(stdout) is str:
            LOG.log(stdout)

        return

    def reboot_device(self):
        '''reboot the remote device'''
        self.adb("reboot")
        self.adb('wait-for-device')
        time.sleep(1)
        self.adb("root")
        time.sleep(1)
        self.adb("remount")
        time.sleep(1)

    def launch_app(self, name, activity):
        '''launch a render script app'''
        assert name and activity

        cmd = 'am start -S -n ' + name + '/' + name + '.' + activity
        stdout = self.shell(cmd)

        if type(stdout) is str:
            LOG.log(stdout)
        return None

    def launch_lldbserver(self, port):
        '''launch lldb server and attach to target app'''
        channels = ["gdb-remote all",
                    "lldb breakpoints",
                    "lldb dyld",
                    "lldb process",
                    "lldb platform"]

        opts = "-l data/lldblog.txt "
        chans = "-c "
        for chan in channels:
            chans += "{0};".format(chan)
        opts += chans

        cmd = "export LLDB_DEBUGSERVER_PATH='{0}';{0} p --listen {1} {2}"\
            .format(self.m_path_lldbserver, port, opts)

        self.shell(cmd, True)
        time.sleep(5)
        return

    def forward_port(self, local, remote):
        '''use adb to forward a device port onto the local machine'''
        # ask adb to forward this port
        cmd = 'forward tcp:' + str(local) + ' tcp:' + str(remote)
        self.adb(cmd)
        return
