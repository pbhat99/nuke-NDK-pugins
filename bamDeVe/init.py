"""
bamDeVe Plugin Initializer

Registers bamDeVe directory with Nuke's pluginPath and Python sys.path.
"""
import os
import sys
import nuke

_this_dir = os.path.dirname(os.path.abspath(__file__))
if _this_dir not in sys.path:
    sys.path.insert(0, _this_dir)

nuke.pluginAddPath(_this_dir)
