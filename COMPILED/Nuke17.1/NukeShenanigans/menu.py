import nuke

toolbar = nuke.menu("Nodes")
shenanigans_menu = toolbar.addMenu("Shenanigans")

shenanigans_menu.addCommand("RemoveUnlimited", "nuke.createNode('RemoveUnlimited')")
shenanigans_menu.addCommand("DeepKeymix", "nuke.createNode('DeepKeymix')")
shenanigans_menu.addCommand("DeepVolumise", "nuke.createNode('DeepVolumise')")
shenanigans_menu.addCommand("ContactSheetAll", "nuke.createNode('ContactSheetAll')")

shenanigans_menu.addSeparator()
shenanigans_menu.addCommand("DeepCount", "nuke.createNode('DeepCount')")
shenanigans_menu.addCommand("DeepHistogram", "nuke.createNode('DeepHistogram')")
shenanigans_menu.addCommand("DeepSampleMerge", "nuke.createNode('DeepSampleMerge')")
shenanigans_menu.addCommand("MirrorExtend", "nuke.createNode('MirrorExtend')")
shenanigans_menu.addCommand("DummyCamera", "nuke.createNode('DummyCamera')")
