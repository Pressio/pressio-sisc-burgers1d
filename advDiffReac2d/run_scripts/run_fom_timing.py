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
  # first col contains mesh sizes, then all timings
  data = np.zeros((numMeshes, constants.numSamplesForTiming+1))

  # args for the executable
  args = ("./"+exename, "input.txt")

  # loop over mesh sizes
  for iMesh in range(0, numMeshes):
    numCell = constants.numCell_cases[iMesh]
    print("Current numCell = ", numCell)

    data[iMesh][0] = numCell

    pathToMeshFile = meshDir + "/mesh" + str(numCell) + ".dat"
    print("Current mesh file = ", pathToMeshFile)

    # create input file (we only need one since the same
    # is used to run multiple replica runs
    myutils.createInputFileFomChemTiming(pathToMeshFile)

    for i in range(0, constants.numSamplesForTiming):
      print("replica # = ", i)
      popen = subprocess.Popen(args, stdout=subprocess.PIPE)
      popen.wait()
      output = popen.stdout.read()

      # find timing
      res = re.search(constants.timerRegExp, str(output))
      time = float(res.group().split()[2])
      # store
      data[iMesh][i+1] = time
      print("time = ", time)

  np.savetxt(exename+"_timings.txt", data, fmt='%.12f')
  print(data)


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe", "--exe", dest="exename")
  parser.add_argument("-mesh-dir", "--mesh-dir", dest="meshdir")
  args = parser.parse_args()
  main(args.exename, args.meshdir)
