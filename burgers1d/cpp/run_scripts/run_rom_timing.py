#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import myutils, constants

#-------------------------------------------------------
# scope: generate timings for C++ Burgers1D rom
#-------------------------------------------------------

def main(exeName, basisDirName):

  # data stored as:
  # - first col  = mesh size
  # - second col = rom size
  # - then       = timings
  nRows = len(constants.numCell_cases) * len(constants.romSize_cases)
  nCols = constants.numSamplesForTiming+2
  data = np.zeros((nRows, nCols))

  # args for the executable
  args = ("./"+exeName, "input.txt")

  # store parent directory where all basis are stored
  basisParentDir = os.getcwd() + "/../data_" + basisDirName

  # loop over mesh sizes
  iRow = -1
  for iMesh in range(0, len(constants.numCell_cases)):
    numCell = constants.numCell_cases[iMesh]
    print("Current numCell = ", numCell)

    # loop over ROM sizes
    for iRom in range(0, len(constants.romSize_cases)):
      # iRow keeps strack of where to store into data
      iRow += 1
      # print current rom size
      romSize = constants.romSize_cases[iRom]
      print("Current romSize = ", romSize)

      # store mesh size and rom size
      data[iRow][0] = numCell
      data[iRow][1] = romSize

      # create input file (we only need one since the same
      # is used to run multiple replicas)
      myutils.createInputFileRom(numCell, romSize)

      # link basis file
      subs1 = "numCell" + str(numCell)
      subs2 = "basis" + str(romSize)
      basisDir = basisParentDir + "/" + subs1 + "/" + subs2
      # always remove the link to basis to make sure we link the right one
      os.system("rm -rf ./basis.txt")
      os.system("ln -s "+basisDir+"/basis.txt ./basis.txt")

      #--- loop over samples ---
      for i in range(0, constants.numSamplesForTiming):
        popen = subprocess.Popen(args, stdout=subprocess.PIPE)
        popen.wait()
        output = popen.stdout.read()
        #print( str(output))
        #os.system("./" + exeName + " input.txt")

        # find timing
        res = re.search(constants.timerRegExp, str(output))
        time = float(res.group().split()[2])
        print("time = ", time)
        # store
        data[iRow][i+2] = time

        # save output data (e.g. state and gen coords) for one replica run only
        # since they are all equivalent, beside the timing
        if i==0:
          destDir = "numCell" + str(numCell) + "/basis" + str(romSize)
          os.system("mkdir -p " + destDir)
          if os.path.isfile('yFomReconstructed.txt'):
            os.system("mv yFomReconstructed.txt " + destDir)
          if os.path.isfile('final_generalized_coords.txt'):
            os.system("mv final_generalized_coords.txt " + destDir)
          os.system("cp input.txt " + destDir)

  np.savetxt(exeName+"_timings.txt", data, fmt='%.12f')
  np.set_printoptions(edgeitems=10, linewidth=100000)
  print(data)


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe", "--exe", dest="exeName")
  parser.add_argument("-basis-dir-name", "--basis-dir-name",
                      dest="basisDirName")
  args = parser.parse_args()
  main(args.exeName, args.basisDirName)
