from config import config
import sys

class rtd_log:

    fd_ = None
    path_ = None

    def __init__(self):
        try:
            self.fd_ = open(config.get_log_file_path() + "/log.txt", "wb")
        except:
            pass

    def __del__(self):
        if self.fd_:
            self.fd_.flush()
            self.fd_.close()

    def writeln(self, msg):
        if not type(msg) is str:
            msg = str(msg)
        self.log("{0}".format(msg))
        print(msg)

    def write(self, msg):
        if not type(msg) is str:
            msg = str(msg)
        self.log(msg)
        sys.stdout.write(msg)

    def log(self, msg):
        if not type(msg) is str:
            msg = str(msg)
        if self.fd_:
            self.fd_.write("{0}\r\n".format(msg))

global log
log = rtd_log()
