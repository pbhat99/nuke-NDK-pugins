import nuke
import roto_reducer

# Node toolbar entry
toolbar = nuke.menu("Nodes")
draw_menu = toolbar.findItem("Draw")
if draw_menu:
    draw_menu.addCommand("RotoReducer", "nuke.createNode('RotoReducer')")
    draw_menu.addCommand("RotoExplode", "nuke.createNode('RotoExplode')")
else:
    toolbar.addCommand("RotoReducer", "nuke.createNode('RotoReducer')")
    toolbar.addCommand("RotoExplode", "nuke.createNode('RotoExplode')")

# Properties panel RMB menu entry
nuke.menu('Properties').addCommand('Roto Reducer...', 'roto_reducer.show_dialog()')
