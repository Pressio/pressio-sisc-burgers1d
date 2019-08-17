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

def main(exename, meshDir, basisDirName):
  numMeshes = len(constants.numCell_cases)

  # data stored as:
  # - first col  = mesh size
  # - second col = rom size
  # - then       = timings
  data = np.zeros((numMeshes, constants.numSamplesForTiming+2))

  # args for the executable
  args = ("./"+exename, "input.txt")

  # store parent directory where all basis are stored
  basisParentDir = os.getcwd() + "/../" + basisDirName

  # loop over mesh sizes
  for iMesh in range(0, numMeshes):
    numCell = constants.numCell_cases[iMesh]
    print("Current numCell = ", numCell)

    pathToMeshFile = meshDir + "/mesh" + str(numCell) + ".dat"
    print("Current mesh file = ", pathToMeshFile)

    # loop over ROM sizes
    for iRom in range(0, len(constants.romSize_cases)):
      romSize = constants.romSize_cases[iRom]
      print("Current romSize = ", romSize)

      data[iMesh][0] = numCell
      data[iMesh][1] = romSize

      # create input file
      myutils.createInputFileFomChemForLSPGFullMesh(pathToMeshFile, romSize)

      # copy basis here
      subs1 = "numCell" + str(numCell)
      subs2 = "basis" + str(romSize)
      basisDir = basisParentDir + "/" + subs1 + "/" + subs2
      os.system("cp "+basisDir+"/basis.txt .")

      for i in range(0, constants.numSamplesForTiming):
        print("replica # = ", i)
        popen = subprocess.Popen(args, stdout=subprocess.PIPE)
        popen.wait()
        output = popen.stdout.read()
        # find timing
        res = re.search(constants.timerRegExp, str(output))
        time = float(res.group().split()[2])
        # store
        data[iMesh][i+2] = time
        print("time = ", time)

        # save data for one replica run
        if i==0:
          destDir = "numCell" + str(numCell) + "/basis" + str(romSize)
          os.system("mkdir -p " + destDir)
          if os.path.isfile('xy.txt'):
            os.system("mv xy.txt " + destDir)
          if os.path.isfile('xFomReconstructed.txt'):
            os.system("mv xFomReconstructed.txt " + destDir)
          os.system("mv input.txt " + destDir)

  np.savetxt(exename+"_timings.txt", data, fmt='%.12f')
  print(data)


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe", "--exe", dest="exename")
  parser.add_argument("-mesh-dir", "--mesh-dir", dest="meshdir")
  parser.add_argument("-basis-dir", "--basis-dir", dest="basisDirName")
  args = parser.parse_args()
  main(args.exename, args.meshdir, args.basisDirName)
