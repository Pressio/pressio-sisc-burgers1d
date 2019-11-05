#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path
import re
from argparse import ArgumentParser

import constants


def main(exename):
  # col0 :      mesh size
  # col1,2...:  all timings
  data = np.zeros( (constants.num_meshes, constants.numSamplesForTiming+1) )

  #----------------------------
  #--- loop over mesh sizes ---
  for iMesh in range(0, constants.num_meshes):

    meshSize = constants.mesh_sizes[iMesh]
    print("Current meshSize = ", meshSize)
    # store the current mesh size
    data[iMesh][0] = meshSize

    # args to run (args changes since each replica
    # is done with different values of inputs)
    if exename=="main_fom_velocity":
      args = ("python", exename+".py", str(meshSize))

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
      data[iMesh][i+1] = time
      print("time = ", time)

  # save to text
  np.savetxt(exename+"_timings.txt", data, fmt='%.12f')
  # make sure the data table is not wrapped over multiple lines
  np.set_printoptions(edgeitems=10, linewidth=100000)
  print(data)

if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-exe", "--exe", dest="exename")
  args = parser.parse_args()
  main(args.exename)
