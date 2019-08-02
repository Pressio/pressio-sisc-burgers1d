#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re

import constants

# regex for getting timing from code output
timerRegExp = re.compile(r'Elapsed time: [0-9].\d{9}')

#-------------------------------------------------------
# scope: generate timings for Python Burgers1D rom
#-------------------------------------------------------

# data stored as:
# - first col  = mesh size
# - second col = rom size
# - then       = timings

nCols = constants.numSamplesForTiming+2
data = np.zeros((len(constants.numCell_cases), nCols))

# store parent directory where all basis are stored
basisParentDir = os.getcwd() + "/../../cpp/fom_basis"

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

    # copy basis here
    subs1 = "numCell" + str(numCell)
    subs2 = "basis" + str(romSize)
    basisDir = basisParentDir + "/" + subs1 + "/" + subs2
    os.system("cp "+basisDir+"/basis.txt .")

    # args to run (args changes since each replica
    # is done with different values of inputs)
    args = ("python", "main_rom.py",
            str(numCell), str(romSize),
            str(constants.numSteps), str(constants.dt))

    for i in range(0, constants.numSamplesForTiming):
      popen = subprocess.Popen(args, stdout=subprocess.PIPE)
      popen.wait()
      output = popen.stdout.read()
      # find timing
      res = re.search(timerRegExp, str(output))
      time = float(res.group().split()[2])
      print("time = ", time)
      # store
      data[iMesh][i+2] = time

#np.savetxt("rom_timings.txt", data, fmt='%.12f')
#print(data)
print ("done")
