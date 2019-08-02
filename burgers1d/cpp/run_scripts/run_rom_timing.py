#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re

import myutils, constants

#-------------------------------------------------------
# scope: generate timings for C++ Burgers1D rom
#-------------------------------------------------------

# data stored as:
# - first col  = mesh size
# - second col = rom size
# - then       = timings

nCols = constants.numSamplesForTiming+2
data = np.zeros((len(constants.numCell_cases), nCols))

# args for the executable
args = ("./burgers1d_rom", "input.txt")

# store parent directory where all basis are stored
basisParentDir = os.getcwd() + "/../fom_basis"

# loop over mesh sizes
for iMesh in range(0, len(constants.numCell_cases)):
  numCell = constants.numCell_cases[iMesh]
  print("Current numCell = ", numCell)

  # loop over ROM sizes
  for iRom in range(0, len(constants.romSize_cases)):
    romSize = constants.romSize_cases[iRom]
    print("Current romSize = ", romSize)

    data[iMesh][0] = numCell
    data[iMesh][1] = romSize

    # create input file (we only need one since the same
    # is used to run multiple replicas)
    myutils.createInputFileRom(numCell, romSize)

    # copy basis here
    subs1 = "numCell" + str(numCell)
    subs2 = "basis" + str(romSize)
    basisDir = basisParentDir + "/" + subs1 + "/" + subs2
    os.system("cp "+basisDir+"/basis.txt .")

    for i in range(0, constants.numSamplesForTiming):
      popen = subprocess.Popen(args, stdout=subprocess.PIPE)
      popen.wait()
      output = popen.stdout.read()
      # find timing
      res = re.search(myutils.timerRegExp, str(output))
      time = float(res.group().split()[2])
      print("time = ", time)
      # store
      data[iMesh][i+2] = time

np.savetxt("rom_timings.txt", data, fmt='%.12f')
print(data)
