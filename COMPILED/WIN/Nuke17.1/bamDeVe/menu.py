"""Nuke menu: adds bam > bamDeVe to the Nodes toolbar and main menu bar."""

import nuke

for _bar in ("Nodes", "Nuke"):
    m = nuke.menu(_bar).findItem("bam")
    if not m:
        m = nuke.menu(_bar).addMenu("bam")
    m.addCommand("bamDeVe", "nuke.createNode('bamDeVe')")
