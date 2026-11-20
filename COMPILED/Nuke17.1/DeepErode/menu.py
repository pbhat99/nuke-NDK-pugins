# menu.py for DeepErode
import nuke

toolbar = nuke.menu("Nodes")
deepMenu = toolbar.findItem("Deep")
if deepMenu:
    deepMenu.addCommand("DeepErode", "nuke.createNode('DeepErode')")
else:
    deepMenu = toolbar.addMenu("Deep")
    deepMenu.addCommand("DeepErode", "nuke.createNode('DeepErode')")
