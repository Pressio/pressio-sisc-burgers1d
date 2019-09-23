#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import myutils_common as utc
import myutils_chem as utchem
import constants_chem as cch

#####################################################################
#####################################################################
# this script run ONLY timings for the FOM adr2d chemistry problem
#####################################################################
#####################################################################

def main(exeName, meshDir, stepperName):
  # how many meshes I have to do
  numMeshes = len(cch.numCell_cases)

  # data is a matrix storing  all results such that:
  # col0:     mesh size
  # col1:     dof for residual vector
  # col2:     dof for state vector
  # col3-end: all timings for all realizations of a given mesh
  data = np.zeros((numMeshes, cch.numSamplesForTiming+3))

  # args for running the executable
  args = ("./"+exeName, "input.txt")

  #------------------------
  # loop over mesh sizes
  for iMesh in range(0, numMeshes):
    numCell = cch.numCell_cases[iMesh]
    print("Current numCell = ", numCell)

    # store the current mesh size into data
    data[iMesh][0] = numCell

    # get the name of the mesh file for the current case
    pathToMeshFile = utc.generateMeshFilePath(meshDir, numCell, numCell, "full")
    print (pathToMeshFile)
    # check if meshfile for current size exists in that directory
    assert( os.path.exists(pathToMeshFile) )
    print("Current mesh file = ", pathToMeshFile)

    # create input file (we only need to create it once,
    # since the same input is used to run multiple replica runs)
    utchem.createInputFileFomChemTiming(stepperName, pathToMeshFile)

    #------------------------------------------------------
    # loop over the number of runs to do for each mesh size
    for i in range(0, cch.numSamplesForTiming):
      print("replica # = ", i+1)
      popen = subprocess.Popen(args, stdout=subprocess.PIPE)
      popen.wait()
      output = popen.stdout.read()
      print (output)

      # if we are at first replica run, grep the number of Dofs
      if i == 0:
        # dofs for residual vector
        res = re.search(utc.numDofResidRegExp, str(output))
        numDofResid = np.int32(res.group().split()[2])
        data[iMesh][1] = numDofResid
        # dofs for state vector
        res = re.search(utc.numDofStateRegExp, str(output))
        numDofState = np.int32(res.group().split()[2])
        data[iMesh][2] = numDofState
        print("dofResid = ", numDofResid)
        print("dofState = ", numDofState)

      # find timing from executable output
      res = re.search(utc.timerRegExp, str(output))
      time = float(res.group().split()[2])
      # store in data
      data[iMesh][i+3] = time
      print("Time = ", time, "\n")

  # save the results to file
  np.savetxt(exeName+"_timings.txt", data, fmt='%.12f')
  np.set_printoptions(edgeitems=10, linewidth=100000)
  print(data)


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe",           "--exe",        dest="exeName")
  parser.add_argument("-mesh-dir",      "--mesh-dir",   dest="meshDir")
  parser.add_argument("-stepper-name", "--stepper-name",dest="stepperName")
  args = parser.parse_args()
  print( args.stepperName )
  main(args.exeName, args.meshDir, args.stepperName)
