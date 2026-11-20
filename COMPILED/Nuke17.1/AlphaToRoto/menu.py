import nuke

toolbar = nuke.menu("Nodes")
draw_menu = toolbar.findItem("Draw")
if draw_menu:
    draw_menu.addCommand("AlphaToRoto", "nuke.createNode('AlphaToRoto')")
else:
    toolbar.addCommand("AlphaToRoto", "nuke.createNode('AlphaToRoto')")
