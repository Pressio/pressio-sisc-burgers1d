#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re

import myutils, constants

# this Python script generates basis from the Burgers1D fom

# args for the executable
args = ("./burgers1d_fom", "input.txt")

# loop over mesh sizes
for iMesh in range(0, len(constants.numCell_cases)):
  numCell = constants.numCell_cases[iMesh]
  print("Current numCell = ", numCell)

  # create folder
  parentDir='numCell' + str(numCell)
  if not os.path.exists(parentDir):
    os.system('mkdir ' + parentDir)

  # loop over various basis size
  for i in range(0, len(constants.romSize_cases)):
    romSize = constants.romSize_cases[i]
    print("Current romSize = ", romSize)

    # based on the size of rom and number of ode steps,
    # compute the sampling frequency
    assert(constants.numSteps % romSize == 0)
    samplingFreq = int(constants.numSteps/romSize)

    # create input file
    myutils.createInputFileFomForBasis(numCell, samplingFreq)

    popen = subprocess.Popen(args, stdout=subprocess.PIPE)
    popen.wait()
    #output = popen.stdout.read()

    childDir=parentDir + '/basis' + str(romSize)
    if not os.path.exists(childDir):
      os.system('mkdir ' + childDir)

    os.system('mv input.txt ' + childDir)
    os.system('mv basis.txt ' + childDir)
    os.system('mv snapshots.txt ' + childDir)
