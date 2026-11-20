"""
CropMore menu.py
Registers CropMore and GetBounds in Nuke's node toolbar.
"""
import nuke

toolbar = nuke.menu("Nodes")

# Add to Transform menu
transform_menu = toolbar.findItem("Transform")
if transform_menu:
    transform_menu.addCommand("CropMore", "nuke.createNode('CropMore')")
else:
    toolbar.addCommand("CropMore", "nuke.createNode('CropMore')")

# Also register GetBounds in Utility / Transform
if transform_menu:
    transform_menu.addCommand("GetBounds", "nuke.createNode('GetBounds')")

