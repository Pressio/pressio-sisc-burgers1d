
import matplotlib.pyplot as plt
import sys, os, time
import numpy as np
from numpy import linspace, meshgrid
from matplotlib import cm
import collections
from argparse import ArgumentParser
import random

powersOfTwo = np.array([5,6])#5,6,7,8,9])
fullMeshCases = 2**powersOfTwo

# list of what percentages of the full mesh we want
smPercentCases = np.array([5,25])#1, 5, 10, 25, 50, 75])

# the ordering for the mesh, options:
# - natural :
# - rcm     : if you want to enable reverse Cuthill-McKee reordering
ordering = "rcm"

#-------- nothing to change below here ----------#

numFM = len(fullMeshCases)
numSM = len(smPercentCases)

# loop over all full meshes
for iMesh in range(numFM):
  nFM = fullMeshCases[iMesh]
  # total num of cells (square grid)
  totCellsFM = nFM*nFM

  print ("Doing full mesh with nFM = ", nFM, " fmSize = ", totCellsFM)

  # running command for creating the FULL mesh first
  cmdLine1 = "python create_single_mesh.py"
  #no need to pass ny for square grid
  cmdLine2 = " --nx "+str(nFM)
  cmdLine3 = " --sampling-type full "
  cmdLine4 = " --ordering " + ordering + " "

  logFileName = "mesh_log_nFM" + str(nFM) + "_full.log"
  os.system(cmdLine1 + cmdLine2 + cmdLine3 + cmdLine4 + " > " + logFileName)

  # check if the mesh directory exists
  meshesParentDir = os.getcwd() + "/meshes_" + ordering
  if not os.path.exists(meshesParentDir):
    os.system("mkdir " + meshesParentDir)

  meshDirName = str(nFM) + "x" + str(nFM)
  if not os.path.exists(meshesParentDir+"/"+meshDirName):
    os.system("mkdir " + meshesParentDir+"/"+meshDirName)

  # check if the subdir containining the full mesh exists
  fullSubdir = meshesParentDir+"/"+meshDirName+"/full"
  if not os.path.exists(fullSubdir):
    os.system("mkdir " + fullSubdir)

  # copy the mesh files into there
  os.system("mv -f ./mesh.dat " + fullSubdir)
  os.system("mv -f " + logFileName + " " + fullSubdir)

  #---------------------------------------------------------
  # given a full mesh, loop over the sample mesh percentages
  # to generate all sample meshes
  for iSM in range(numSM):
    # get what percent this SM wants
    thisSMPercent = smPercentCases[iSM]
    print ("Doing case with nFM = ", nFM,
           " fmSize = ", totCellsFM,
           " and SM % = ", thisSMPercent)

    # running command for creating the SAMPLE mesh first
    cmdL1 = "python create_single_mesh.py"
    cmdL2 = " --nx "+str(nFM) + " --ny "+str(nFM)
    cmdL3 = " --sampling-type random "
    cmdL4 = " --pct " + str(thisSMPercent)
    cmdL5 = " --ordering " + ordering + " "
    logFileName = "mesh_log_nFM" + str(nFM) + "_full.log"
    os.system(cmdL1 + cmdL2 + cmdL3 + cmdL4 + cmdL5 + " > " + logFileName)

    # check if the mesh directory exists
    meshDirName = str(nFM) + "x" + str(nFM)
    if not os.path.exists(meshesParentDir+"/"+meshDirName):
      os.system("mkdir " + meshesParentDir+"/"+meshDirName)

    # check if the subdir containining the sample mesh exists
    subdir = meshesParentDir+"/"+meshDirName+"/sample_pct" + str(thisSMPercent)
    if not os.path.exists(subdir):
      os.system("mkdir " + subdir)

    # copy the mesh files into there
    os.system("mv -f ./mesh.dat " + subdir)
    os.system("mv -f ./sm_to_fm_gid_mapping.dat " + subdir)
    os.system("mv -f " + logFileName + " " + subdir)
