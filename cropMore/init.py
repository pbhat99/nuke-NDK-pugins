"""
CropMore init.py
Ensures crop_more module and CropMore gizmo / GetBounds plugin are on sys.path and pluginPath.
"""
import os
import sys
import nuke

this_dir = os.path.dirname(os.path.abspath(__file__))

if this_dir not in sys.path:
    sys.path.append(this_dir)

nuke.pluginAddPath(this_dir)

