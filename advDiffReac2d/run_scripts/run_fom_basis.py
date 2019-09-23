#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import myutils_common as utc
import myutils_chem as utchem
import constants_chem as cchem

################################################################
################################################################
# script running basis computation for chemistry adr2d
################################################################
################################################################

def main(exeName, meshDir, stepperName):
  # the mesh sizes I want to loop over
  numMeshes = len(cchem.numCell_cases)
  # the rom sizes I want to loop over
  numRomSizes = len(cchem.romSize_cases)

  # args for the executable
  args = ("./"+exeName, "input.txt")

  #-------------------------
  # loop over mesh sizes
  for iMesh in range(0, numMeshes):
    numCell = cchem.numCell_cases[iMesh]
    print("Current numCell = ", numCell)

    # get the name of the mesh file for the current case
    pathToMeshFile = utc.generateMeshFilePath(meshDir, numCell, numCell, "full")
    print (pathToMeshFile)
    # check if meshfile for current size exists in that directory
    assert( os.path.exists(pathToMeshFile) )
    print("Current mesh file = ", pathToMeshFile)

    # create folder for current mesh size
    parentDir='numCell' + str(numCell)
    if not os.path.exists(parentDir):
      os.system('mkdir ' + parentDir)

    # loop over the target basis sizes
    for i in range(0, numRomSizes):
      romSize = cchem.romSize_cases[i]
      print("Current romSize = ", romSize)

      # skip cases where num basis is > time steps
      if romSize > cchem.numSteps:
        print(" skipping case with meshSize=", numCell, " and romSize=", romSize)
        print(" because romSize > numSteps. you know I cannot do this.")
      else:
        # based on the size of rom and number of ode steps,
        # compute the sampling frequency
        assert(cchem.numSteps % romSize == 0)
        samplingFreq = np.int32(cchem.numSteps/romSize)
        print ("Sampling freq = ", samplingFreq)

        # create input file
        utchem.createInputFileFomChemForBasis(stepperName, pathToMeshFile, samplingFreq)

        os.system("./" + exeName + " input.txt")
        #popen = subprocess.Popen(args, stdout=subprocess.PIPE)
        #popen.wait()
        #output = popen.stdout.read()

        # create a directory to save the output for current romSize
        childDir=parentDir + '/basis' + str(romSize)
        if not os.path.exists(childDir): os.system('mkdir ' + childDir)
        # save all outputs to new directory
        os.system('mv input.txt ' + childDir)
        os.system('mv basis.txt ' + childDir)
        os.system('mv snapshots.txt ' + childDir)
        os.system('mv xy.txt ' + childDir)

  print("Done with basis runs")


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe",           "--exe",          dest="exeName")
  parser.add_argument("-mesh-dir",      "--mesh-dir",     dest="meshdir")
  parser.add_argument("-stepper-name",  "--stepper-name", dest="stepperName")
  args = parser.parse_args()
  main(args.exeName, args.meshdir, args.stepperName)
