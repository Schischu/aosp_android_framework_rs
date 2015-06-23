'''Module that contains the Log class.'''

import sys
from config import Config

class Log(object):
    '''A log class for printing more detailed output to a file
       during execution of the test suite.'''

    fd_ = None # Handle to the log file

    def __init__(self):
        self.fd_ = open(Config.get_log_file_path() + '/log.txt', 'wb')

    def __del__(self):
        if self.fd_:
            self.fd_.flush()
            self.fd_.close()

    def writeln(self, msg):
        '''Write a message to stdout and the log file,
           and append a new line character

        Args:
            msg: The string that is the message to log.
        '''
        if not type(msg) is str:
            msg = str(msg)
        self.log('{0}'.format(msg))
        print msg

    def write(self, msg):
        '''Write a message to stdout and the log file

        Args:
            msg: The string that is the message to log.
        '''
        if not type(msg) is str:
            msg = str(msg)
        self.log(msg)
        sys.stdout.write(msg)

    def log(self, msg):
        '''Write a message to the log file

        Args:
            msg: The string that is the message to log.
        '''
        if not type(msg) is str:
            msg = str(msg)
        if self.fd_:
            self.fd_.write('{0}\r\n'.format(msg))

LOG = Log()
