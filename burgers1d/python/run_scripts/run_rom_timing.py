#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import constants

#-------------------------------------------------------
# scope: generate timings for Python Burgers1D rom
#-------------------------------------------------------

def main(exename, basisDirName):
  # data stored as:
  # - first col  = mesh size
  # - second col = rom size
  # - then       = timings

  nCols = constants.numSamplesForTiming+2
  data = np.zeros((len(constants.numCell_cases), nCols))

  # store parent directory where all basis are stored
  basisParentDir = os.getcwd() + "/../../cpp/data_" + basisDirName

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
      # always remove the link to basis to make sure we link the right one
      os.system("rm -rf ./basis.txt")
      os.system("ln -s "+basisDir+"/basis.txt ./basis.txt")

      # args to run (args changes since each replica
      # is done with different values of inputs)
      args = ("python", exename+".py",
              str(numCell), str(romSize),
              str(constants.numSteps), str(constants.dt))

      for i in range(0, constants.numSamplesForTiming):
        # popen = subprocess.Popen(args, stdout=subprocess.PIPE)
        # popen.wait()
        # output = popen.stdout.read()
        # print(output)
        os.system("python "+exename+".py"+" "+
                  str(numCell)+" "+
                  str(romSize)+" "+
                  str(constants.numSteps)+" "+
                  str(constants.dt) )

        # find timing
        #res = re.search(constants.timerRegExp, str(output))
        time = -1 #float(res.group().split()[2])
        print("time = ", time)
        # store
        data[iMesh][i+2] = time

  np.savetxt(exename+"_timings.txt", data, fmt='%.12f')
  print(data)


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe", "--exe", dest="exename")
  parser.add_argument("-basis-dir-name", "--basis-dir-name",
                      dest="basisDirName")
  args = parser.parse_args()
  main(args.exename, args.basisDirName)
