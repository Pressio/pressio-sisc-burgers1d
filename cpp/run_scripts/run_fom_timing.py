#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import myutils, constants

def main(exename):
  # col0 :      mesh size
  # col1,2...:  all timings
  data = np.zeros( (constants.num_meshes, constants.numSamplesForTiming+1) )

  # args for the executable
  args = ("./"+exename, "input.txt")

  #----------------------------
  #--- loop over mesh sizes ---
  for iMesh in range(0, constants.num_meshes):

    currentMeshSize = constants.mesh_sizes[iMesh]
    print("Current currentMeshSize = ", currentMeshSize)
    # store the current mesh size
    data[iMesh][0] = currentMeshSize

    # create input file (we only need one since the same
    # is used to run multiple replica runs
    myutils.createInputFileFomTiming(currentMeshSize)

    # --- loop over replicas runs ---
    for i in range(0, constants.numSamplesForTiming):
      print("replica # = ", i)

      # run with subprocess
      popen = subprocess.Popen(args, stdout=subprocess.PIPE)
      popen.wait()
      # get output
      output = popen.stdout.read()

      # find timing
      res = re.search(constants.timerRegExp, str(output))
      time = float(res.group().split()[2])
      # store time for this replica
      data[iMesh][i+1] = time/float(constants.numStepsTiming[currentMeshSize])
      print("time = ", time)

  # save to text
  np.savetxt(exename+"_timings.txt", data, fmt='%.15f')
  # make sure the data table is not wrapped over multiple lines
  np.set_printoptions(edgeitems=10, linewidth=100000)
  print(data)


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe", "--exe", dest="exename",
                      help="run timings for fom")
  args = parser.parse_args()
  main(args.exename)
