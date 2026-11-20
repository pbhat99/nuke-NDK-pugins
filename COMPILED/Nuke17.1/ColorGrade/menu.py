# menu.py for ColorGrade
import nuke

toolbar = nuke.menu("Nodes")
colorMenu = toolbar.findItem("Color")
if colorMenu:
    colorMenu.addCommand("ColorGrade", "nuke.createNode('ColorGrade')")
else:
    toolbar.addCommand("Color/ColorGrade", "nuke.createNode('ColorGrade')")
