"""
ImageTool menu.py
Registers ImageTool in Nuke's node toolbar under Color.
"""
import nuke

toolbar = nuke.menu("Nodes")
color_menu = toolbar.findItem("Color")
if color_menu:
    color_menu.addCommand("ImageTool", "nuke.createNode('ImageTool')")
else:
    toolbar.addCommand("ImageTool", "nuke.createNode('ImageTool')")
