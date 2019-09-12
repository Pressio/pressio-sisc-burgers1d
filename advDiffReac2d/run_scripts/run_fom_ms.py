#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import myutils_common as utc
import myutils_ms as utms
import constants_ms as cms

################################################################
################################################################
# this script tests accuracy and convergence of the
# adr for the manufactued solution
################################################################
################################################################

def main(exeName, meshDir, stepperName):

  # data is a matrix that contains:
  # col 0      : contains mesh size
  # col 1,2,3  : l2 norm of error for species 0,1,2
  # col 4,5,6  : linf norm of error for species 0,1,2
  data = np.zeros((len(cms.numCell_cases), 7))

  #---------------------------------
  # loop over meshes
  for iMesh in range(0, len(cms.numCell_cases)):
    numCell = cms.numCell_cases[iMesh]
    print("Current numCell = ", numCell)

    # get the name of the mesh file for the current case
    meshFileName = utc.generateMeshFileName(numCell, numCell, "full")
    # the path where mesh file is located
    pathToMeshFile = meshDir + "/" + meshFileName
    # check if meshfile for current size exists in that directory
    assert( os.path.exists(pathToMeshFile) )
    print("Current mesh file = ", pathToMeshFile)

    # create input file for the FOM manuf sol problem
    utms.createInputFileFomEigenMs(stepperName, pathToMeshFile)

    #------------------
    # run the exe
    #------------------
    os.system("./" + exeName + " input.txt")

    #------------------
    # process solution
    #------------------
    # load solution at final time, which we get from snapshots file
    snaps = np.loadtxt("snapshots.txt")
    finalPressioSol = snaps[:,-1]
    # separate state for all three species
    [c0,c1,c2,c0rs,c1rs,c2rs] = utc.splitStateBySpecies(finalPressioSol, numCell)

    # extract time to which the final column of the snapshots corresponds to
    finalTime = utc.extractFinalTimeFromTargetInputFile(".")

    # load coords (these are printed by the cpp executable)
    coords = np.loadtxt("xy.txt")
    x,y = coords[:,0], coords[:,1]
    # compute analytical solution for the target time
    [u0,u1,u2] = cms.msTrueSolution(x, y, finalTime)

    #------------------
    # STORING results:
    #------------------
    # store the mesh size into data matrix
    data[iMesh][0] = numCell

    # compute RMS error for each species
    data[iMesh][1] = np.sqrt( ((u0 - c0) ** 2).mean() )
    data[iMesh][2] = np.sqrt( ((u1 - c1) ** 2).mean() )
    data[iMesh][3] = np.sqrt( ((u2 - c2) ** 2).mean() )

    # compute lInf norm error for each species
    data[iMesh][4] = np.max( np.abs(u0 - c0) )
    data[iMesh][5] = np.max( np.abs(u1 - c1) )
    data[iMesh][6] = np.max( np.abs(u2 - c2) )

    #------------------
    # save run outputs:
    #------------------
    # create folder for the case that was just run
    tmpDir="mesh" + str(numCell)

    os.system("mkdir " + tmpDir)
    # copy data of this run into the folder
    os.system("mv input.txt " + tmpDir)
    os.system("mv snapshots.txt " + tmpDir)
    os.system("mv xy.txt " + tmpDir)

  #--------------------------
  # Save data matrix to file:
  np.savetxt(exeName+"_errors.txt", data, fmt='%.12f')
  np.set_printoptions(edgeitems=10, linewidth=100000)
  print(data)



if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe",           "--exe",        dest="exeName")
  parser.add_argument("-mesh-dir",      "--mesh-dir",   dest="meshDir")
  parser.add_argument("-stepper-name", "--stepper-name",dest="stepperName")
  args = parser.parse_args()
  main(args.exeName, args.meshDir, args.stepperName)
