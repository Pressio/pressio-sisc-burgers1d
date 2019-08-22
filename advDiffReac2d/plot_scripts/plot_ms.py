#!/usr/bin/env python

import matplotlib.pyplot as plt
import sys, os, time
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
from sklearn.metrics import mean_squared_error
from math import sqrt
import os.path
from numpy import linspace, meshgrid
from matplotlib.mlab import griddata
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D
from argparse import ArgumentParser

from constants_ms import msTrueSolution
from myutils_common import splitStateBySpecies, computeTimeOfSnapshot
from myutils_common import extractDtFromTargetInputFile, extractSamplingFreqFromTargetInputFile
from plot_common import loadXY


def plotSingleSnapshot(n, xrs, yrs, snapId, snaps, sampFreq, dt):
  # get only target state from snapshot matrix
  targetState = snaps[:, snapId]

  # split state by species
  [c0,c1,c2,c0rs,c1rs,c2rs] = splitStateBySpecies(targetState, n)

  # calculate the time this snapshot corresponds to
  snapshotTime = computeTimeOfSnapshot(snapId, sampFreq, dt)

  # compute true manufactured solution at that time
  [u0,u1,u2] = msTrueSolution(x,y, snapshotTime)
  u0rs = u0.reshape(n,n)
  u1rs = u1.reshape(n,n)
  u2rs = u2.reshape(n,n)

  print("\n")
  print ("max/min true u0 ", np.min(u0), np.max(u0) )
  print ("max/min c0 "     , np.min(c0), np.max(c0))
  print ("error = "        , np.sqrt(((u0 - c0) ** 2).mean()))
  print("\n")
  print ("max/min true u1 ", np.min(u1), np.max(u1) )
  print ("max/min c1 "     , np.min(c1), np.max(c1))
  print ("error = "        , np.sqrt(((u1 - c1) ** 2).mean()))
  print("\n")
  print ("max/min true u2 ", np.min(u2), np.max(u2) )
  print ("max/min c2 "     , np.min(c2), np.max(c2))
  print ("error = "        , np.sqrt(((u2 - c2) ** 2).mean()))
  print("\n")

  fig = plt.figure(0)
  ax = fig.add_subplot(111, projection='3d')
  ax.plot_wireframe(xrs, yrs, u0rs, rstride=1, cstride=1, color='r')
  ax.plot_wireframe(xrs, yrs, c0rs, rstride=1, cstride=1, color='k')
  fig = plt.figure(1)
  ax = fig.add_subplot(111, projection='3d')
  ax.plot_wireframe(xrs, yrs, u1rs, rstride=1, cstride=1, color='r')
  ax.plot_wireframe(xrs, yrs, c1rs, rstride=1, cstride=1, color='k')
  fig = plt.figure(2)
  ax = fig.add_subplot(111, projection='3d')
  ax.plot_wireframe(xrs, yrs, u2rs, rstride=1, cstride=1, color='r')
  ax.plot_wireframe(xrs, yrs, c2rs, rstride=1, cstride=1, color='k')
  plt.show()


if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-n", "--n", type=int, dest="n", help="number of cells along one axis")
  parser.add_argument("-data-dir", "--data-dir", type=str,
                      dest="dataDir", help="target data directory")
  parser.add_argument("-from-snap-id", "--from-snap-id", dest="startFrom", type=int)
  parser.add_argument("-to-snap-id", "--to-snap-id", dest="endAt", type=int)
  parser.add_argument("-jump", "--jump", dest="freq", type=int)
  args = parser.parse_args()

  # load xy
  [x, y, xrs, yrs] = loadXY(args.dataDir, args.n)

  # load snapshots
  snaps = np.loadtxt(args.dataDir+"/snapshots.txt")

  # grab sampling frequency and dt from the input file
  dt = extractDtFromTargetInputFile(args.dataDir)
  samplingFrequency = extractSamplingFreqFromTargetInputFile(args.dataDir)

  if (int(args.freq) == 0):
    plotSingleSnapshot(args.n, xrs, yrs, args.startFrom, snaps, samplingFrequency, dt)
