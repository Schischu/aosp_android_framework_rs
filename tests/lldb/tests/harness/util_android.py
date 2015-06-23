'''Module that contains the class UtilAndroid,
   providing utility method to interface with Android ADB.'''

import subprocess
import time

from exception import TestSuiteException

class UtilAndroid(object):
    '''Provides some utility methods that interface with Android using adb.'''

    m_path_adb = '' # The path to the adb binary on the local machine
    m_path_lldbserver = ''  # The path to the lldb server binary on the device

    @staticmethod
    def _validate_string(string):
        '''Check that a string is valid and not empty

        Args:
            string: The string to be checked.
        '''
        assert type(string) is str
        assert len(string) > 0

    def __init__(self, log, config):
        assert log
        assert config
        self.m_path_adb = config.get_adb_path()
        self.m_path_lldbserver = config.get_lldb_server_path()
        self.log_ = log
        return

    def adb(self, args, async=False):
        '''run an adb command (async optional)

        Args:
            args: The command (including arguments) to run in adb
            async: Boolean to specify whether adb should run the command
                asynchronously.

        Returns:
            If adb was synchronously run, a string which is the output (standard
            out and error) from adb. Otherwise None.
        '''
        # form the command
        if self.m_path_adb:
            cmd = self.m_path_adb + ' ' + args
        else:
            cmd = 'adb {0}'.format(args)
        self.log_.log(cmd)

        # spawn async thread
        if async:
            subprocess.Popen(cmd,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             shell=True)
            return None

        # spawn blocking
        else:
            # execute the command
            proc = subprocess.Popen(cmd,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    shell=True)
            (stdout, stderr) = proc.communicate(None)
            self.log_.log(stdout)
            self.log_.log(stderr)
            return stdout + stderr

    def get_abi(self):
        '''return the device abi

        Returns:
            A string, which is the output of the command
            adb shell 'getprop ro.product.cpu.abi'
        '''
        return self.shell('getprop ro.product.cpu.abi')

    def shell(self, cmd, async=False):
        '''run something via the adb shell

        Args:
            args: The command (including arguments) to run in the adb shell
            async: Boolean to specify whether adb should run the command
                asynchronously.

        Returns:
            If adb was synchronously run, a string which is the output (standard
            out and error) from adb. Otherwise None.
        '''
        return self.adb('shell "{0}"'.format(cmd), async)

    def find_app_pid(self, process_name):
        '''find an app pid

        Args:
            process_name: A string representing the name of the package or
                binary for which the id should be found. I.e. the string or part
                of the string that shows up in the "ps" command.

        Returns:
            An integer representing the id of the process, or None if it was not
            found.
        '''
        self._validate_string(process_name)
        stdout = self.shell('ps | grep {0}'.format(process_name))

        # reduce white space to single space
        stdout = stdout.replace('\t', ' ')
        for _ in range(1, 5):
            stdout = stdout.replace('  ', ' ')

        items = stdout.split(' ')
        if len(items) <= 1:
            self.log_.log('unable to find pid of: {0}'.format(process_name))
            return None

        try:
            pid = int(items[1])
            self.log_.log('app pid found: {0}'.format(items[1]))
            return pid
        except ValueError:
            return None

    def adb_root(self):
        '''Set adb to be in root mode'''
        self.adb('root')

    def adb_remount(self):
        '''Remount filesystem'''
        self.adb('remount')

    def validate_adb(self):
        '''Validate adb that it can be run.

        Raises:
            TestSuiteException: Unable to validate that adb exists and
            runs successfully.
        '''
        out = self.adb('version')
        if out and 'Android' in out and 'version' in out:
            self.log_.log('adb found: {0}'.format(out))
            return None
        raise TestSuiteException('unable to validate adb')

    def validate_device(self):
        '''Validate that there is at least one device.

        Raises:
            TestSuiteException: There was a failure to run adb to list the
            devices, or there is no device or multiple devices connected
            to the host machine (and accessible to adb).
        '''
        out = self.adb('devices')
        if not 'List of devices attached' in out:
            raise TestSuiteException('Unable to list devices')

        found_device = False

        lines = out.split('\n')
        for line in lines[1:]:
            if '\tdevice' in line:
                if found_device:
                    raise TestSuiteException(
                        'adb found multiple devices. '
                        'This is currently not supported by the test suite. '
                        'Please disconnect all but one.')

                found_device = True

        if not found_device:
            raise TestSuiteException('adb is unable to find a connected '
                                     'device/emulator to test.')

    def kill_pid(self, pid):
        '''Kill a process identified by its pid by issuing a "kill" command

        Args:
            pid: The integer that is the process id of the process to be killed.
        '''
        self.shell('kill ' + str(pid))

    def stop_app(self, package_name):
        '''Terminate an app by calling am force-stop

        Args:
            package_name: The string representing the name of the package of the
                app that is to be stopped.
        '''
        self._validate_string(package_name)
        self.shell('am force-stop ' + package_name)

    def kill_process(self, name):
        '''Kill a process identified by its name
           (package name in case of apk) by issuing a "kill" command

        Args:
            name: The string representing the name of the binary of the
                process that is to be killed.

        Returns:
            True if the kill command was executed, False if it could not be
            found.
        '''
        pid = self.find_app_pid(name)
        if pid:
            self.kill_pid(pid)
            return True
        return False

    def kill_all_processes(self, name):
        '''Repeatedly try to call "kill" on a process to ensure it is gone.
           If the process is still there after 5 attempts reboot the device.

        Args:
            name: The string representing the name of the binary of the
                process that is to be killed.
        '''

        # try 5 times to kill this process
        for _ in range(1, 5):
            if not self.kill_process(name):
                return None
        # stalled process must reboot
        self.reboot_device()

    def clean_device(self):
        '''Clean the device state,
           killing all gdbserver and lldb-server instances.'''
        self.kill_all_processes('gdbserver')
        self.kill_all_processes('lldb-server')

    def launch_elf(self, binary_name, args, env):
        '''Launch a binary (compiled with the NDK)

        Args:
            binary_name: The string representing the name of the binary that is
                to be launched.
            args: The string or list of strings representing the arguments to
                the binary.
            env: A string->string dictionary of environment variables and their
                values that is to be exported prior to launching the binary.
        '''
        export_statements = []
        for (key, value) in env:
            export_statements.append('export {0} ="{1}";'.format(key, value))

        cmd = ' '.join(export_statements) + 'exec /data/' + binary_name

        if type(args) is str:
            cmd += args

        if type(args) is list:
            cmd += ' '.join(args)

        stdout = self.shell('"{0}"'.format(cmd), True)
        if type(stdout) is str:
            self.log_.log(stdout)

    def wait_for_device(self):
        '''Ask ADB to wait for a device to become ready'''
        self.adb('wait-for-device')

    def reboot_device(self):
        '''reboot the remote device'''
        self.adb('reboot')
        self.adb('wait-for-device')
        time.sleep(1)
        self.adb('root')
        time.sleep(1)
        self.adb('remount')
        time.sleep(1)

    def launch_app(self, name, activity):
        '''Launch a render script app

        Args:
            name: The string representing the name of the app that is to be
                launched.
            activity: The string representing the activity of the app that is to
                be started.
        '''
        assert name and activity

        cmd = 'am start -S -n {0}/{0}.{1}'.format(name, activity)
        stdout = self.shell(cmd)

        if type(stdout) is str:
            self.log_.log(stdout)

    def launch_lldb_platform(self, port):
        '''Launch lldb server and attach to target app

        Args:
            port: The integer that is the port on which lldb should listen.
        '''
        cmd = "export LLDB_DEBUGSERVER_PATH='{0}';{0} p --listen {1}"\
            .format(self.m_path_lldbserver, port)
        self.shell(cmd, True)
        time.sleep(5)

    def forward_port(self, local, remote):
        '''Use adb to forward a device port onto the local machine

        Args:
            local: The integer that is the local port to forward.
            remote: The integer that is the remote port to which to forward.
        '''
        cmd = 'forward tcp:%s tcp:%s' % (str(local), str(remote))
        self.adb(cmd)
