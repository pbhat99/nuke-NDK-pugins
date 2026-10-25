# OptiXDenoiser menu script for Foundry Nuke
import nuke

toolbar = nuke.menu('Nodes')
filterMenu = toolbar.findItem('Filter')
if filterMenu:
    filterMenu.addCommand('OptiXDenoiser', 'nuke.createNode("OptiXDenoiser")')
else:
    toolbar.addCommand('Filter/OptiXDenoiser', 'nuke.createNode("OptiXDenoiser")')
