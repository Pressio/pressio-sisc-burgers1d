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

  # number of FULL meshe sizes
  numFullMeshes = len(cchem.numCell_cases)

  # number of sample meshes
  numSampleMeshes = len(cchem.sampleMesh_sizes)

  # various number of basis
  numRomSizes = len(cchem.romSize_cases)

  # the total number of meshes I have to run is:
  numCombinations = numFullMeshes * numSampleMeshes * numRomSizes
  # data stored as:
  # col=0  :  full mesh size
  # col=1  :  number of cells in sample mesh where we compute residual
  # col=2  :  num of dof for residual vector
  # col=3  :  num of dof for state vector
  # col=4  :  rom size
  # col=5: :  timings
  data = np.zeros((numCombinations, cchem.numSamplesForTiming+5))

  # args for the executable
  args = ("./"+exeName, "input.txt")

  # store parent directory where all basis are stored
  basisParentDir = os.getcwd() + "/../" + basisDirName

  # keeping track of which row to store in data
  rowCount = -1

  # ------------------------
  # loop over FULL meshes
  # ------------------------
  for iFullMesh in range(0, numFullMeshes):

    numCellFull = cchem.numCell_cases[iFullMesh]
    print("Current full mesh numCell = ", numCellFull)

    #-------------------------
    # loop over SAMPLE mesh sizes
    #-------------------------
    # for each full mesh, I loop over the various sample mesh sizes
    for iSampleMesh in range(0, len(cchem.sampleMesh_sizes)):

      thisSMSize = cchem.sampleMesh_sizes[iSampleMesh]
      print("Current sample mesh size = ", thisSMSize)

      # get the name of the mesh file for the current case
      pathToMeshFile = utc.generateMeshFilePath(meshDir,numCellFull,numCellFull,
                                                "random", thisSMSize)
      print (pathToMeshFile)
      # check if meshfile for current size exists in that directory
      assert( os.path.exists(pathToMeshFile) )
      print("Current mesh file = ", pathToMeshFile)

      # get the name of the file with mapping of GIDs from SM to FM
      pathToGIDsMappingFile = utc.generateSmToFmGIDsMapFilePath(meshDir,numCellFull,
                                                                numCellFull,
                                                                "random", thisSMSize)
      print (pathToGIDsMappingFile)
      # check if meshfile for current size exists in that directory
      assert( os.path.exists(pathToGIDsMappingFile) )
      print("Current GIDs mapping file = ", pathToGIDsMappingFile)

      #-------------------------
      # loop over ROM sizes
      #-------------------------
      for iRom in range(0, numRomSizes):
        rowCount += 1

        # the current ROM size
        romSize = cchem.romSize_cases[iRom]
        print("Current romSize = ", romSize)

        # store the current FULL mesh size into data
        data[rowCount][0] = numCellFull
        # store the current SAMPLE mesh size into data
        data[rowCount][1] = thisSMSize

        # create input file
        utchem.createInputFileFomChemForLSPGSampleMesh(stepperName,
                                                       pathToMeshFile,
                                                       romSize,
                                                       pathToGIDsMappingFile)

        # copy basis here
        subs1 = "numCell" + str(numCellFull)
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
            data[rowCount][2] = numDofResid
            # dofs for state vector
            res = re.search(utc.numDofStateRegExp, str(output))
            numDofState = int(res.group().split()[2])
            data[rowCount][3] = numDofState
            print("dofResid = ", numDofResid)
            print("dofState = ", numDofState)
            # store the rom size
            data[rowCount][4] = romSize

          # # find timing for current run
          # res = re.search(utc.timerRegExp, str(output))
          # time = float(res.group().split()[2])
          # store
          data[rowCount][i+5] = -1.0 #time
          print("time = ", time)

          # save data for one replica run only
          if i==0:
            destDirp1 = "numCellFull" + str(numCellFull)
            destDirp2 = "_smSize" + str(thisSMSize)
            destDirp3 = "_basis" + str(romSize)
            destDir= destDirp1 + destDirp2 + destDirp3;
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
