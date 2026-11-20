# menu.py for FastNoiseLite
import nuke

toolbar = nuke.menu("Nodes")
drawMenu = toolbar.findItem("Draw")
if drawMenu:
    drawMenu.addCommand("FastNoiseLite", "nuke.createNode('FastNoiseLite')")
else:
    toolbar.addCommand("Draw/FastNoiseLite", "nuke.createNode('FastNoiseLite')")
