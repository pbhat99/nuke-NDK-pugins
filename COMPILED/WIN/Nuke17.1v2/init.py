"""
Nuke Nuke17.1v2 (WIN) Plugin Auto-Discovery Initializer
Registers all tool subdirectories with Nuke's pluginPath.
"""
import os
import sys
import nuke

_root = os.path.dirname(os.path.abspath(__file__))
for _item in sorted(os.listdir(_root)):
    _sub = os.path.join(_root, _item)
    if os.path.isdir(_sub) and not _item.startswith((".", "_")):
        if _sub not in sys.path:
            sys.path.insert(0, _sub)
        nuke.pluginAddPath(_sub)
