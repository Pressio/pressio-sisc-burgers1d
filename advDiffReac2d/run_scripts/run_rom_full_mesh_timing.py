#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import myutils_common as utc
import myutils_chem as utchem
import constants_chem as cchem


def main(exeName, meshDir, stepperName, basisDirName):
  # number of cases for mesh,romSize, of which I ran several replicas
  numCases = len(cchem.numCell_cases) * len(cchem.romSize_cases)

  numMeshes = len(cchem.numCell_cases)

  # data stored as:
  # col=0  :  mesh size
  # col=1  :  num of dof for residual vector
  # col=2  :  num of dof for state vector
  # col=3  :  rom size
  # col=4: :  timings
  data = np.zeros((numCases, cchem.numSamplesForTiming+4))

  # args for the executable
  args = ("./"+exeName, "input.txt")

  # store parent directory where all basis are stored
  basisParentDir = os.getcwd() + "/../" + basisDirName

  rowCount = -1

  # loop over mesh sizes
  for iMesh in range(0, numMeshes):
    numCell = cchem.numCell_cases[iMesh]
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
    for iRom in range(0, len(cchem.romSize_cases)):
      rowCount+=1

      romSize = cchem.romSize_cases[iRom]
      print("Current romSize = ", romSize)

      # store the current mesh size into data
      data[rowCount][0] = numCell

      # create input file
      utchem.createInputFileFomChemForLSPGFullMesh(stepperName, pathToMeshFile, romSize)

      # copy basis here
      subs1 = "numCell" + str(numCell)
      subs2 = "basis" + str(romSize)
      basisDir = basisParentDir + "/" + subs1 + "/" + subs2
      # always remove the link to basis to make sure we link the right one
      os.system("rm -rf ./basis.txt")
      os.system("ln -s "+basisDir+"/basis.txt ./basis.txt")

      #-------------------------
      # do all the replicas
      #-------------------------
      for i in range(0, cchem.numSamplesForTiming):
        print("replica # = ", i)
        os.system("./" + exeName + " input.txt")
        # popen = subprocess.Popen(args, stdout=subprocess.PIPE)
        # popen.wait()
        # output = popen.stdout.read()

        # if we are at first replica run, grep the number of Dofs
        if i == 0:
          # dofs for residual vector
          res = re.search(utc.numDofResidRegExp, str(output))
          numDofResid = int(res.group().split()[2])
          data[rowCount][1] = numDofResid
          # dofs for state vector
          res = re.search(utc.numDofStateRegExp, str(output))
          numDofState = int(res.group().split()[2])
          data[rowCount][2] = numDofState
          print("dofResid = ", numDofResid)
          print("dofState = ", numDofState)
          # store the rom size
          data[rowCount][3] = romSize

        # find timing for current run
        res = re.search(utc.timerRegExp, str(output))
        time = float(res.group().split()[2])
        # store
        data[rowCount][i+4] = time
        print("time = ", time)

        # save data for one replica run only
        if i==0:
          destDir = "numCell" + str(numCell) + "/basis" + str(romSize)
          os.system("mkdir -p " + destDir)
          if os.path.isfile('xy.txt'):
            os.system("mv xy.txt " + destDir)
          if os.path.isfile('xFomReconstructed.txt'):
            os.system("mv xFomReconstructed.txt " + destDir)
          os.system("mv input.txt " + destDir)

  np.savetxt(exeName+"_timings.txt", data, fmt='%.12f')
  print(data)


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe",           "--exe",        dest="exeName")
  parser.add_argument("-mesh-dir",      "--mesh-dir",   dest="meshdir")
  parser.add_argument("-stepper-name", "--stepper-name",dest="stepperName")
  parser.add_argument("-basis-dir",     "--basis-dir",  dest="basisDirName")
  args = parser.parse_args()
  main(args.exeName, args.meshdir, args.stepperName, args.basisDirName)
