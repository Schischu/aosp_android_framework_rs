# Defines the exception that is thrown whenever an internal error is encountered. Just contains a message.
class exception:

    msg_ = ""

    # Constructor. Create an exception with a given message.
    def __init__(self, message):
        self.msg_ = message

    # Get the message of the exception.
    def get(self):
        return self.msg_

    # Get the message of the exception.
    def __repr__(self):
        return self.msg_
