#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import myutils, constants

def main(exeName, language):

  # data stored as:
  # - first  col  = numDofs
  # - second col  = romSize
  # - third col   = timing
  # - fourth col  = gflop
  # - fifth col   = gflop/sec
  nRows = len(constants.numDofs) * len(constants.romSizes)
  nCols = 5
  data = np.zeros((nRows, nCols))

  # args for the executable
  argsCpp = ("./"+exeName, "input.txt")


  # loop over rows of A matrix
  kk = -1
  for iRowsA in range(0, len(constants.numDofs)):
    matArows = constants.numDofs[iRowsA]
    print("Current numDofs = ", matArows)

    # loop over column sizes
    for iCols in range(0, len(constants.romSizes)):
      # iRow keeps strack of where to store into data
      kk += 1

      # print current rom size
      romSize = constants.romSizes[iCols]
      print("Current rom size = ", romSize)

      # store
      data[kk][0] = matArows
      data[kk][1] = romSize

      if language=="Cpp":
        myutils.createInputFile(matArows, romSize, constants.numReplicas)

      # args to run (args changes since each replica
      # is done with different values of inputs)
      argsPy = ("python", exeName+".py",
                str(matArows), str(romSize),
                str(constants.numReplicas) )

      if language=="Python":
        args = argsPy
      else:
        args = argsCpp

      popen = subprocess.Popen(args, stdout=subprocess.PIPE)
      popen.wait()
      output = popen.stdout.read()
      #print( str(output))
      #os.system("./" + exeName + " input.txt")

      # find timing
      res = re.search(constants.timerRegExp, str(output))
      time = float(res.group().split()[2])
      print("Total time = ", time)
      # find timing per replic
      res = re.search(constants.avgTimeRegExp, str(output))
      time2 = float(res.group().split()[4])
      print("Time per item = ", time2)

      # find gflop
      res = re.search(constants.gflopRegExp, str(output))
      gflop = float(res.group().split()[1])
      print("GFlopPerItem = ", gflop)
      # find GFlop/sec
      res = re.search(constants.gflopsRegExp, str(output))
      gflopsec = float(res.group().split()[1])
      print("GFlop/sec = ", gflopsec)

      data[kk][2] = time
      data[kk][3] = gflop
      data[kk][4] = gflopsec
      print("\n")

  np.savetxt(exeName+"_stats.txt", data, fmt='%.12f')
  np.set_printoptions(edgeitems=10, linewidth=100000)
  print(data)


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe", "--exe", dest="exeName")
  parser.add_argument("-lang", "--lang", dest="language")
  args = parser.parse_args()
  main(args.exeName, args.language)
