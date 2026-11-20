import os
import sys
import nuke

plugin_dir = os.path.dirname(os.path.realpath(__file__))
lib_dir = os.path.join(plugin_dir, 'lib')
plugins_dir = os.path.join(plugin_dir, 'plugins')
oidn_bin = r'C:\oidn\bin'

# Add directories to Nuke's plugin search path
nuke.pluginAddPath(plugin_dir)
if os.path.isdir(plugins_dir):
    nuke.pluginAddPath(plugins_dir)
if os.path.isdir(lib_dir):
    nuke.pluginAddPath(lib_dir)

# On Windows, ensure dependent DLLs can be resolved by Windows dynamic linker
if sys.platform == 'win32':
    dll_paths = [plugins_dir, lib_dir, oidn_bin, plugin_dir]
    path_entries = [p for p in dll_paths if os.path.isdir(p)]
    if path_entries:
        os.environ['PATH'] = os.pathsep.join(path_entries) + os.pathsep + os.environ.get('PATH', '')
    
    if hasattr(os, 'add_dll_directory'):
        for p in path_entries:
            try:
                os.add_dll_directory(p)
            except Exception:
                pass
