
import matplotlib.pyplot as plt
import sys, os, time
import numpy as np
from numpy import linspace, meshgrid
from matplotlib import cm
import collections
from argparse import ArgumentParser
import random


def main(powersOfTwo, fullMeshCases, smPercentCases, ordering):
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


###############################
if __name__== "__main__":
###############################
  powersOfTwo = np.array([7,8,9,10])
  fullMeshCases = 2**powersOfTwo
  # list of what percentages of the full mesh we want
  smPercentCases = np.array([1, 5, 10, 25, 50])#1, 5, 10, 25, 50, 75])

  parser = ArgumentParser()
  parser.add_argument("-ordering", "--ordering",
                      dest="orderingType",
                      default="natural",
                      help="What type of ordering you want:\n"+
                           "use <natural> for natural ordering of the mesh cells,\n"+
                           "use <rcm> for reverse cuthill-mckee")
  args = parser.parse_args()
  assert( args.orderingType == "natural" or args.orderingType == "rcm" )
  print("*************************************")
  print("Ordering type = " + args.orderingType)
  print("*************************************")
  print("")
  main(powersOfTwo, fullMeshCases, smPercentCases, args.orderingType)
