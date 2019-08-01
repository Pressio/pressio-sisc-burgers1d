#!/usr/bin/env python

import sys, os, time
import subprocess
#from subprocess import Popen, list2cmdline, PIPE
import numpy as np
import os.path
import re

import myutils, constants

# this Python script generates many runs of the Burgers1D fom
# for various configurations and processses results
# this is purely FOM timings, no snapshots are created

# first col contains mesh sizes, then all timings
data = np.zeros((len(constants.numCell_cases), constants.numSamplesForTiming+1))

# args for the executable
args = ("./burgers1d_fom", "input.txt")

# loop over mesh sizes
for iMesh in range(0, len(constants.numCell_cases)):
  numCell = constants.numCell_cases[iMesh]
  print("Current numCell = ", numCell)

  # create input file (we only need one since the same
  # is used to run multiple replica runs
  myutils.createInputFileFomTiming(numCell)

  data[iMesh][0] = numCell

  for i in range(0, constants.numSamplesForTiming):
    popen = subprocess.Popen(args, stdout=subprocess.PIPE)
    popen.wait()
    output = popen.stdout.read()

    # find timing
    res = re.search(myutils.timerRegExp, str(output))
    time = float(res.group().split()[2])
    # store
    data[iMesh][i+1] = time

np.savetxt("fom_timings.txt", data, fmt='%.12f')
print(data)
