#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import constants

def main(exename, basisDirName, denseJac):

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

    # get the number of steps to do for this case
    currNumSteps = constants.numStepsTiming[meshSize]

    # loop over ROM sizes
    for iRom in range(0, constants.num_rom_sizes):
      iRow+=1
      romSize = constants.rom_sizes[iRom]
      print("Current romSize = ", romSize)

      data[iRow][0], data[iRow][1] = meshSize, romSize

      # link basis here
      subs1 = "meshSize" + str(meshSize)
      subs2 = "basis" + str(romSize)
      basisDir = basisParentDir + "/" + subs1 + "/" + subs2
      # always remove the link to basis to make sure we link the right one
      os.system("rm -rf ./basis.txt")
      os.system("ln -s "+basisDir+"/basis.txt ./basis.txt")

      # args to run (args changes since each replica
      # is done with different values of inputs)
      argsLspg = ("python", exename+".py",
                  str(meshSize), str(romSize),
                  str(currNumSteps), str(constants.dt), str(denseJac))

      argsGalerkin = ("python", exename+".py",
                      str(meshSize), str(romSize),
                      str(currNumSteps), str(constants.dt))

      for i in range(0, constants.numSamplesForTiming):
        if ("lspg" in exename):
          popen = subprocess.Popen(argsLspg, stdout=subprocess.PIPE)
        else:
          popen = subprocess.Popen(argsGalerkin, stdout=subprocess.PIPE)

        popen.wait()
        output = popen.stdout.read()

        # find timing
        res = re.search(constants.timerRegExp, str(output))
        time = float(res.group().split()[2])
        print("time = ", time)
        # store
        data[iRow][i+2] = time/float(currNumSteps)

        # while running, overwrite the timings
        timingFile = exename+"_timings.txt"
        if os.path.isfile(timingFile):
          os.system("rm -rf " + timingFile)
        np.savetxt(timingFile, data, fmt='%.15f')

        # save output data (e.g. state and gen coords) for one replica run only
        # since they are all equivalent, beside the timing
        if i==0:
          destDir = "meshSize" + str(meshSize) + "/basis" + str(romSize)
          os.system("mkdir -p " + destDir)
          if os.path.isfile('yFomReconstructed.txt'):
            os.system("mv yFomReconstructed.txt " + destDir)
          if os.path.isfile('final_generalized_coords.txt'):
            os.system("mv final_generalized_coords.txt " + destDir)

  # write timings file
  timingFile = exename+"_timings.txt"
  if os.path.isfile(timingFile):
    os.system("rm -rf " + timingFile)
  np.savetxt(timingFile, data, fmt='%.15f')

  np.set_printoptions(edgeitems=10, linewidth=100000)
  print(data)


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe", "--exe", dest="exename")
  parser.add_argument("-basis-dir-name", "--basis-dir-name", dest="basisDirName")
  parser.add_argument("-dense-jac", "--dense-jac", dest="denseJac")
  args = parser.parse_args()
  main(args.exename, args.basisDirName, args.denseJac)
