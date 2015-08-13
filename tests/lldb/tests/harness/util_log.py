'''Module that contains the UtilLog class.'''

import sys

class UtilLog(object):
    '''A log class for printing more detailed output to a file
       during execution of the test suite.'''
    fd_ = None  # Handle to the log file
    path_ = None # Path to the log file on disk

    def __init__(self):
        pass

    def __del__(self):
        self.close()

    def close(self):
        '''Flush and close the log file, if it is open.'''
        if self.fd_:
            self.fd_.flush()
            self.fd_.close()

    def open(self, path, clear):
        '''Create an on disk copy of the log

        Args:
            path: String, path where the log file should be created
            clear: Boolean, clear the log file if it already exists

        Raises:
            AssertionError: If the path argument is not a string
        '''

        assert path
        assert type(path) is str
        self.path_ = path
        attr = 'a'
        if clear:
            attr = 'w'
        try:
            self.fd_ = open('{0}/log.txt'.format(path), attr)
        except IOError:
            self.writeln('unable to create log file on disk at ' + path)

    def reopen(self):
        '''Reopen an already existing log in append mode.

        Raises:
            AssertionError: If the log file has not previously been opened.
        '''
        self.open(self.path_, False)

    def writeln(self, msg):
        '''Write a message to stdout and the log file,
           and append a new line character

        Args:
            msg: String, message to append to the log.
        '''
        if not type(msg) is str:
            msg = str(msg)
        self.log('{0}'.format(msg))
        print msg

    def write(self, msg):
        '''Write a message to stdout and the log file

        Args:
            msg: String, message to append to the log.
        '''
        if not type(msg) is str:
            msg = str(msg)
        self.log(msg)
        sys.stdout.write(msg)

    def log(self, msg):
        '''Write a message to the log file and flush it.

        Args:
            msg: String, message to append to the log.
        '''
        if not type(msg) is str:
            msg = str(msg)
        if self.fd_:
            self.fd_.write('{0}\n'.format(msg))
            self.fd_.flush()
