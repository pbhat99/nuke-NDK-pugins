import nuke

toolbar = nuke.menu("Nodes")
m3d = toolbar.findItem("3D")
if m3d:
    modify = m3d.findItem("Modify")
    if modify:
        modify.addCommand("USDSubD", "nuke.createNode('USDSubD')")
    else:
        m3d.addCommand("USDSubD", "nuke.createNode('USDSubD')")
else:
    toolbar.addCommand("USDSubD", "nuke.createNode('USDSubD')")
