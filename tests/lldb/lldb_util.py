from log import log
try:
    import lldb
except ImportError as e:
    log.println("unable to import lldb")
    log.println("please run \"lldb -P\" and add to $PYTHONPATH")
    import os
    quit()

# Provides utility methods to interface with lldb's python bindings.
class lldb_util:

    # Initialise the lldb debugger framework
    @staticmethod
    def start():
        lldb.SBDebugger_Initialize()

    # Terminate the lldb debugger framework
    @staticmethod
    def stop():
        lldb.SBDebugger_Terminate()

    # Create an lldb debugger instance
    @staticmethod
    def create_debugger():
        inst = lldb.SBDebugger_Create()
        inst.SetAsync(False)
        return inst

    # Destroy the lldb debugger instance
    @staticmethod
    def destroy_debugger(dbg):
        lldb.SBDebugger_Destroy(dbg)

    # Get the lldb module
    @staticmethod
    def get_module():
        return lldb
