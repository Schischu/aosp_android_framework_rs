'''This file contains utility functions used by both the test suite
   and the single test executor'''

import os

def load_py_module(path):
    '''Load a python file from disk

    Args:
        path: String path to python file

    returns:
        python module if success, None otherwise
    '''
    assert type(path) is str
    try:
        if not os.path.exists(path):
            return None
        path = os.path.abspath(path)
        module_dir, module_file = os.path.split(path)
        module_name, _ = os.path.splitext(module_file)
        save_cwd = os.getcwd()
        os.chdir(module_dir)
        module_obj = __import__(module_name)
        module_obj.__file__ = path
        os.chdir(save_cwd)
        return module_obj
    except ImportError as _:
        return None
