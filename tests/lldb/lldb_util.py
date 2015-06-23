'''Module that contains the class LLDBUtil,
   which provides lldb utility methods.'''

from log import LOG

try:
    import lldb
except ImportError as ex:
    LOG.writeln("unable to import lldb")
    LOG.writeln("please run \"lldb -P\" and add to $PYTHONPATH")
    quit()

class LLDBUtil(object):
    '''Provides utility methods to interface with lldb's python bindings.'''

    # Empty constructor
    def __init__(self):
        pass

    @staticmethod
    def start():
        '''Initialise the lldb debugger framework'''
        lldb.SBDebugger_Initialize()

    @staticmethod
    def stop():
        '''Terminate the lldb debugger framework'''
        lldb.SBDebugger_Terminate()

    @staticmethod
    def create_debugger():
        '''Create an lldb debugger instance'''
        inst = lldb.SBDebugger_Create()
        inst.SetAsync(False)
        return inst

    @staticmethod
    def destroy_debugger(dbg):
        '''Destroy the lldb debugger instance'''
        lldb.SBDebugger_Destroy(dbg)

    @staticmethod
    def get_module():
        '''Get the lldb module'''
        return lldb
