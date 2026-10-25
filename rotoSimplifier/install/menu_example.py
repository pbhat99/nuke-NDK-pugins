import nuke

# 1. Import the module
import roto_simplifier

# 2. Add it to the "Properties" Right-Click Menu
# This makes it appear when you RMB on any Properties panel background.
nuke.menu('Properties').addCommand('Roto Simplifier...', 'roto_simplifier.start()')
