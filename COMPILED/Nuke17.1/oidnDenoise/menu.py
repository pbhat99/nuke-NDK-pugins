import os
import sys
import nuke

# add DLLs to NUKE_PATH
plugin_path = os.path.dirname(os.path.realpath(__file__))
lib_dir_name = 'lib64' if os.name == 'posix' else 'lib'
lib_path = os.path.join(plugin_path, lib_dir_name)
plugins_path = os.path.join(plugin_path, 'plugins')

nuke.pluginAddPath(lib_path)
nuke.pluginAddPath(plugins_path)

# On Windows with Python 3.8+, register DLL directories so dependencies load cleanly
if sys.platform == 'win32' and hasattr(os, 'add_dll_directory'):
    if os.path.isdir(plugins_path):
        os.add_dll_directory(plugins_path)
    if os.path.isdir(lib_path):
        os.add_dll_directory(lib_path)
    if os.path.isdir(r'C:\oidn\bin'):
        os.add_dll_directory(r'C:\oidn\bin')

# Add denoiser entry to the menu
nuke.menu('Nodes').addCommand('oidnDenoise', lambda: nuke.createNode('oidnDenoise'), icon="icon.png")
