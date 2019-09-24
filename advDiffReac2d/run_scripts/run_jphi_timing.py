#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import myutils_common as utc
import myutils_jphi as utjp
import constants_jphi as cjp


def main(exeName, meshDir):
  # number of cases for mesh,romSize, of which I ran several replicas
  numCases = len(cjp.numCell_cases) * len(cjp.romSize_cases)

  numMeshes = len(cjp.numCell_cases)

  # data stored as:
  # col=0  :  mesh size
  # col=1  :  num of dof for residual vector
  # col=2  :  num of dof for state vector
  # col=3  :  rom size, which is equal to the # of basis vector
  # col=4: :  timings
  data = np.zeros((numCases, cjp.numSamplesForTiming+4))

  # args for the executable
  args = ("./"+exeName, "input.txt")

  rowCount = -1
  # loop over mesh sizes
  for iMesh in range(0, numMeshes):
    numCell = cjp.numCell_cases[iMesh]
    print("Current numCell = ", numCell)

    # get the name of the mesh file for the current case
    pathToMeshFile = utc.generateMeshFilePath(meshDir, numCell, numCell, "full")
    print (pathToMeshFile)
    # check if meshfile for current size exists in that directory
    assert( os.path.exists(pathToMeshFile) )
    print("Current mesh file = ", pathToMeshFile)

    #-------------------------
    # loop over ROM sizes
    #-------------------------
    for iRom in range(0, len(cjp.romSize_cases)):
      rowCount+=1

      romSize = cjp.romSize_cases[iRom]
      print("Current romSize = ", romSize)

      # store the current mesh size into data
      data[rowCount][0] = numCell

      # create input file
      utjp.createInputFileGemm(pathToMeshFile, romSize)

      #-------------------------
      # do all the replicas
      #-------------------------
      for i in range(0, cjp.numSamplesForTiming):
        print("replica # = ", i)
        os.system("./" + exeName + " input.txt")
        # popen = subprocess.Popen(args, stdout=subprocess.PIPE)
        # popen.wait()
        # output = popen.stdout.read()

        # # if we are at first replica run, grep the number of Dofs
        # if i == 0:
        #   # dofs for residual vector
        #   res = re.search(utc.numDofResidRegExp, str(output))
        #   numDofResid = np.int32(res.group().split()[2])
        #   data[rowCount][1] = numDofResid
        #   # dofs for state vector
        #   res = re.search(utc.numDofStateRegExp, str(output))
        #   numDofState = np.int32(res.group().split()[2])
        #   data[rowCount][2] = numDofState
        #   print("dofResid = ", numDofResid)
        #   print("dofState = ", numDofState)
        #   # store the rom size
        #   data[rowCount][3] = romSize

        # find timing for current run
        #res = re.search(utc.timerRegExp, str(output))
        time = 0.0 #float(res.group().split()[2])
        # store
        data[rowCount][i+4] = time
        print("time = ", time)

  np.savetxt(exeName+"_timings.txt", data, fmt='%.12f')
  print(data)


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe",           "--exe",        dest="exeName")
  parser.add_argument("-mesh-dir",      "--mesh-dir",   dest="meshdir")
  args = parser.parse_args()
  main(args.exeName, args.meshdir)
