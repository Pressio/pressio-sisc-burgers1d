#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import myutils_ms as myutils
import constants_ms as cms

# this Python script tests both accuracy and convergence
# of the adr for the manufactued solution

def main(exename, meshDir):
  # col 0      : contains mesh size
  # col 1,2,3  : l2 norm of error for species 0,1,2
  # col 4,5,6  : linf norm of error for species 0,1,2
  data = np.zeros((len(cms.numCell_cases), 7))

  # args for the executable
  args = ("./"+exename, "input.txt")

  # loop over meshes
  for iMesh in range(0, len(cms.numCell_cases)):
    numCell = cms.numCell_cases[iMesh]
    print("Current numCell = ", numCell)

    data[iMesh][0] = numCell

    pathToMeshFile = meshDir + "/mesh" + str(numCell) + ".dat"
    print("Current mesh file = ", pathToMeshFile)
    # check if meshfile exists
    assert( os.path.exists(pathToMeshFile) )

    # create input file for the ms fom problem
    myutils.createInputFileFomEigenMs(pathToMeshFile)

    # run exe
    os.system("./" + exename + " input.txt")
    #popen = subprocess.Popen(args, stdout=subprocess.PIPE)
    #popen.wait()
    #output = popen.stdout.read()

    # load solution at final time, which we get from snapshots file
    snaps = np.loadtxt("snapshots.txt")
    finalPressioSol = snaps[:,-1]
    # separate state for all three species
    c0 = finalPressioSol[0:-1:3]
    c1 = finalPressioSol[1:-1:3]
    c2 = finalPressioSol[2:len(finalPressioSol)+1:3]

    # the final col of the snapshots corresponds to the following time
    finalTime = cms.finalTime

    # load coords
    coords = np.loadtxt("xy.txt")
    x,y = coords[:,0], coords[:,1]
    # compute analytical solution
    [u0,u1,u2] = cms.msTrueSolution(x, y, finalTime)

    # compute l2 norm error for each species
    data[iMesh][1] = np.sqrt(((u0 - c0) ** 2).mean())
    data[iMesh][2] = np.sqrt(((u1 - c1) ** 2).mean())
    data[iMesh][3] = np.sqrt(((u2 - c2) ** 2).mean())

    # compute lInf norm error for each species
    data[iMesh][4] = np.max( np.abs(u0 - c0) )
    data[iMesh][5] = np.max( np.abs(u1 - c1) )
    data[iMesh][6] = np.max( np.abs(u2 - c2) )

    # create folder for the case just run
    tmpDir="mesh" + str(numCell)
    os.system("mkdir " + tmpDir)
    # copy data of this run into the folder
    os.system("mv input.txt " + tmpDir)
    os.system("mv snapshots.txt " + tmpDir)
    os.system("mv xy.txt " + tmpDir)

  np.savetxt(exename+"_errors.txt", data, fmt='%.12f')
  print(data)


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe", "--exe", dest="exename")
  parser.add_argument("-mesh-dir", "--mesh-dir", dest="meshdir")
  args = parser.parse_args()
  main(args.exename, args.meshdir)
