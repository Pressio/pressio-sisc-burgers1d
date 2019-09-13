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
  # col0:     mesh size,
  # col1-end: all timings for all realizations of a given mesh
  data = np.zeros((numMeshes, cch.numSamplesForTiming+1))

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
    meshFileName = utc.generateMeshFileName(numCell, numCell, "full")
    # the path where mesh file is located
    pathToMeshFile = meshDir + "/" + meshFileName
    print("Current mesh file = ", pathToMeshFile)
    assert( os.path.exists(pathToMeshFile) )

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

      # find timing from executable output
      res = re.search(cch.timerRegExp, str(output))
      time = float(res.group().split()[2])
      # store in data
      data[iMesh][i+1] = time
      print("time = ", time, "\n")

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
