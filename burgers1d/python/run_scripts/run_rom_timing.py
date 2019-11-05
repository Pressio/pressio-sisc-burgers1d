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
  nRows = constants.num_meshes * constants.num_rom_sizes
  nCols = constants.numSamplesForTiming+2
  data = np.zeros((nRows, nCols))

  # store parent directory where all basis are stored
  basisParentDir = os.getcwd() + "/../../cpp/data_" + basisDirName

  # loop over mesh sizes
  iRow = -1
  for iMesh in range(0, constants.num_meshes):
    meshSize = constants.mesh_sizes[iMesh]
    print("Current meshSize = ", meshSize)

    # loop over ROM sizes
    for iRom in range(0, constants.num_rom_sizes):
      iRow+=1
      romSize = constants.rom_sizes[iRom]
      print("Current romSize = ", romSize)

      data[iRow][0] = meshSize
      data[iRow][1] = romSize

      # link basis here
      subs1 = "meshSize" + str(meshSize)
      subs2 = "basis" + str(romSize)
      basisDir = basisParentDir + "/" + subs1 + "/" + subs2
      # always remove the link to basis to make sure we link the right one
      os.system("rm -rf ./basis.txt")
      os.system("ln -s "+basisDir+"/basis.txt ./basis.txt")

      # args to run (args changes since each replica
      # is done with different values of inputs)
      args = ("python", exename+".py",
              str(meshSize), str(romSize),
              str(constants.numSteps), str(constants.dt))

      for i in range(0, constants.numSamplesForTiming):
        popen = subprocess.Popen(args, stdout=subprocess.PIPE)
        popen.wait()
        output = popen.stdout.read()
        #print(output)
        # os.system("python "+exename+".py"+" "+
        #           str(meshSize)+" "+
        #           str(romSize)+" "+
        #           str(constants.numSteps)+" "+
        #           str(constants.dt) )

        # find timing
        res = re.search(constants.timerRegExp, str(output))
        time = float(res.group().split()[2])
        print("time = ", time)
        # store
        data[iRow][i+2] = time

  np.savetxt(exename+"_timings.txt", data, fmt='%.12f')
  np.set_printoptions(edgeitems=10, linewidth=100000)
  print(data)


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe", "--exe", dest="exename")
  parser.add_argument("-basis-dir-name", "--basis-dir-name",
                      dest="basisDirName")
  args = parser.parse_args()
  main(args.exename, args.basisDirName)
