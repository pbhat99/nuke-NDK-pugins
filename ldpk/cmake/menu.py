# LDPK (Lens Distortion Plugin Kit - 3DEqualizer) menu script for Foundry Nuke
import nuke

toolbar = nuke.menu("Nodes")
m = toolbar.addMenu("3DE4")

# Standard models
m.addCommand("LD_3DE4_Anamorphic_Standard_Degree_4", "nuke.createNode('LD_3DE4_Anamorphic_Standard_Degree_4')")
m.addCommand("LD_3DE4_Anamorphic_Rescaled_Degree_4", "nuke.createNode('LD_3DE4_Anamorphic_Rescaled_Degree_4')")
m.addCommand("LD_3DE4_Anamorphic_Standard_Degree_6", "nuke.createNode('LD_3DE4_Anamorphic_Standard_Degree_6')")
m.addCommand("LD_3DE4_Anamorphic_Rescaled_Degree_6", "nuke.createNode('LD_3DE4_Anamorphic_Rescaled_Degree_6')")
m.addCommand("LD_3DE4_Anamorphic_Stabilizer_Degree_6", "nuke.createNode('LD_3DE4_Anamorphic_Stabilizer_Degree_6')")
m.addCommand("LD_3DE4_Anamorphic_Degree_6", "nuke.createNode('LD_3DE4_Anamorphic_Degree_6')")
m.addCommand("LD_3DE4_Radial_Standard_Degree_4", "nuke.createNode('LD_3DE4_Radial_Standard_Degree_4')")
m.addCommand("LD_3DE4_Radial_Stabilizer_Degree_6", "nuke.createNode('LD_3DE4_Radial_Stabilizer_Degree_6')")
m.addCommand("LD_3DE4_Radial_Fisheye_Degree_8", "nuke.createNode('LD_3DE4_Radial_Fisheye_Degree_8')")
m.addCommand("LD_3DE_Classic_LD_Model", "nuke.createNode('LD_3DE_Classic_LD_Model')")

# Experimental & Fisheye models
m.addCommand("LD_3DE4_Radial_Fisheye_Equidistant_Degree_8", "nuke.createNode('LD_3DE4_Radial_Fisheye_Equidistant_Degree_8')")
m.addCommand("LD_3DE4_Radial_Fisheye_Equisolid_Degree_8", "nuke.createNode('LD_3DE4_Radial_Fisheye_Equisolid_Degree_8')")
m.addCommand("LD_3DE4_Radial_Fisheye_Orthographic_Degree_8", "nuke.createNode('LD_3DE4_Radial_Fisheye_Orthographic_Degree_8')")
m.addCommand("LD_3DE4_Radial_Fisheye_Stereographic_Degree_8", "nuke.createNode('LD_3DE4_Radial_Fisheye_Stereographic_Degree_8')")
m.addCommand("LD_3DE4_Anamorphic_Homomorphic_Degree_2", "nuke.createNode('LD_3DE4_Anamorphic_Homomorphic_Degree_2')")
m.addCommand("LD_3DE4_Radial_Homomorphic_Degree_2", "nuke.createNode('LD_3DE4_Radial_Homomorphic_Degree_2')")
m.addCommand("LD_3DE4_Brown_Conrady_Degree_6_2", "nuke.createNode('LD_3DE4_Brown_Conrady_Degree_6_2')")
m.addCommand("LD_OpenCV_Radial_Degree_6_Decentered_Degree_2", "nuke.createNode('LD_OpenCV_Radial_Degree_6_Decentered_Degree_2')")
