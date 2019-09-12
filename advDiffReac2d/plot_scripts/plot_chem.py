
import matplotlib.pyplot as plt
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
from math import sqrt
import os.path
from numpy import linspace, meshgrid
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D
from argparse import ArgumentParser

import constants_chem as cch

from myutils_common import splitStateBySpecies, computeTimeOfSnapshot
from myutils_common import extractDtFromTargetInputFile
from myutils_common import extractSamplingFreqFromTargetInputFile
from myutils_common import extractFinalTimeFromTargetInputFile

from plot_common import loadXY


def plotSingleSnapshot(n, xrs, yrs, snapId, snaps, samplingFreq, dt):
  # get only target state from snapshot matrix
  targetState = snaps[:, snapId]

  # split state by species
  [c0,c1,c2,c0rs,c1rs,c2rs] = splitStateBySpecies(targetState, n)

  # calculate the time this snapshot corresponds to
  snapshotTime = computeTimeOfSnapshot(snapId, samplingFreq, dt)

  print("-------")
  print ("max/min c0 "     , np.min(c0), np.max(c0))
  print ("max/min c1 "     , np.min(c1), np.max(c1))
  print ("max/min c2 "     , np.min(c2), np.max(c2))
  print("\n")

  fig = plt.figure(1)
  ax1 = fig.add_subplot(131)
  ax2 = fig.add_subplot(132)
  ax3 = fig.add_subplot(133)

  ax1.imshow(c0rs, cmap=cm.jet, origin='lower',interpolation='bicubic')
  ax1.get_xaxis().set_visible(False)
  ax1.get_yaxis().set_visible(False)
  ax1.set_aspect(aspect=1)

  ax2.imshow(c1rs, cmap=cm.brg, origin='lower',interpolation='bicubic')
  ax2.get_xaxis().set_visible(False)
  ax2.get_yaxis().set_visible(False)
  ax2.set_aspect(aspect=1)

  ax3.imshow(c2rs, cmap=cm.terrain, origin='lower',interpolation='bicubic')
  ax3.get_xaxis().set_visible(False)
  ax3.get_yaxis().set_visible(False)
  ax3.set_aspect(aspect=1)
  plt.show()


def plotSequence(n, xrs, yrs, fromSnapId, toSnapId, snaps, samplingFreq, dt):

  for i in range(fromSnapId, toSnapId+1):
    # get target state from snapshot matrix
    targetState = snaps[:, i]

    # split state by species
    [c0,c1,c2,c0rs,c1rs,c2rs] = splitStateBySpecies(targetState, n)

    # calculate the time this snapshot corresponds to
    snapshotTime = computeTimeOfSnapshot(i, samplingFreq, dt)

    print("-------")
    print ("max/min c0 "     , np.min(c0), np.max(c0))
    print ("max/min c1 "     , np.min(c1), np.max(c1))
    print ("max/min c2 "     , np.min(c2), np.max(c2))
    print("\n")

    fig = plt.figure(1)
    ax1 = fig.add_subplot(131)
    ax2 = fig.add_subplot(132)
    ax3 = fig.add_subplot(133)

    ax1.imshow(c0rs, cmap=cm.jet, origin='lower',interpolation='bicubic')
    ax1.get_xaxis().set_visible(False)
    ax1.get_yaxis().set_visible(False)
    ax1.set_aspect(aspect=1)

    ax2.imshow(c1rs, cmap=cm.brg, origin='lower',interpolation='bicubic')
    ax2.get_xaxis().set_visible(False)
    ax2.get_yaxis().set_visible(False)
    ax2.set_aspect(aspect=1)

    ax3.imshow(c2rs, cmap=cm.terrain, origin='lower',interpolation='bicubic')
    ax3.get_xaxis().set_visible(False)
    ax3.get_yaxis().set_visible(False)
    ax3.set_aspect(aspect=1)
    plt.show()



if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-n", "--n", type=int, dest="n", help="number of cells along one axis")
  parser.add_argument("-data-dir", "--data-dir", type=str, dest="dataDir", help="target data directory")
  parser.add_argument("-from-snap-id", "--from-snap-id", dest="startFrom", type=int)
  parser.add_argument("-to-snap-id", "--to-snap-id", dest="endAt", type=int)
  parser.add_argument("-jump", "--jump", dest="freq", type=int)
  args = parser.parse_args()

  assert( int(args.freq) >= 0)

  # load xy
  [x, y, xrs, yrs] = loadXY(args.dataDir, args.n)

  # load snapshots
  snaps = np.loadtxt(args.dataDir+"/snapshots.txt")

  # grab dt from the input file
  dt = extractDtFromTargetInputFile(args.dataDir)
  # grab sampling frequency from the input file
  samplingFrequency = extractSamplingFreqFromTargetInputFile(args.dataDir)
  # grab findlTime from the input file
  finalTime = extractFinalTimeFromTargetInputFile(args.dataDir)
  print ("total steps = ", finalTime/dt)

  if (int(args.freq) == 0):
    plotSingleSnapshot(args.n, xrs, yrs, args.startFrom, snaps, samplingFrequency, dt)
  elif (int(args.freq) > 0):
    plotSequence(args.n, xrs, yrs, args.startFrom, args.endAt, snaps, samplingFrequency, dt)
