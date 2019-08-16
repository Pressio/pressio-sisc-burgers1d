#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import myutils_chem as myutils
import constants_chem as constants

# this Python script computes timings for chem adr2d

def main(exename, meshDir):
  numMeshes = len(constants.numCell_cases)
  numRomSizes = len(constants.romSize_cases)

  # args for the executable
  args = ("./"+exename, "input.txt")

  # loop over mesh sizes
  for iMesh in range(0, numMeshes):
    numCell = constants.numCell_cases[iMesh]
    print("Current numCell = ", numCell)

    pathToMeshFile = meshDir + "/mesh" + str(numCell) + ".dat"
    print("Current mesh file = ", pathToMeshFile)

    # create folder
    parentDir='numCell' + str(numCell)
    if not os.path.exists(parentDir):
      os.system('mkdir ' + parentDir)

    # loop over various basis size
    for i in range(0, numRomSizes):
      romSize = constants.romSize_cases[i]
      print("Current romSize = ", romSize)

      # based on the size of rom and number of ode steps,
      # compute the sampling frequency
      assert(constants.numSteps % romSize == 0)
      samplingFreq = int(constants.numSteps/romSize)

      # create input file
      myutils.createInputFileFomChemForBasis(pathToMeshFile, samplingFreq)

      popen = subprocess.Popen(args, stdout=subprocess.PIPE)
      popen.wait()
      #output = popen.stdout.read()

      childDir=parentDir + '/basis' + str(romSize)
      if not os.path.exists(childDir): os.system('mkdir ' + childDir)

      os.system('mv input.txt ' + childDir)
      os.system('mv basis.txt ' + childDir)
      os.system('mv snapshots.txt ' + childDir)
      os.system('mv xy.txt ' + childDir)

  print("Done with basis runs")


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe", "--exe", dest="exename")
  parser.add_argument("-mesh-dir", "--mesh-dir", dest="meshdir")
  args = parser.parse_args()
  main(args.exename, args.meshdir)
