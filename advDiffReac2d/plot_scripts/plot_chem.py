
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


def computeBounds(c0,c1,c2):
  return {'c0': [np.min(c0), np.max(c0)],
          'c1': [np.min(c1), np.max(c1)],
          'c2': [np.min(c2), np.max(c2)]}

def printBounds(dic):
  print ("max/min c0 "     , dic['c0'][0], dic['c0'][1])
  print ("max/min c1 "     , dic['c1'][0], dic['c1'][1])
  print ("max/min c2 "     , dic['c2'][0], dic['c2'][1])
  print("\n")


def plotScatter(x, y, state, ax, title, cmName, vminIn, vmaxIn):
  ax.set_title(title, fontsize=12)
  ax.scatter(x, y, c=state, cmap=cmName, marker='s',
             edgecolors='face', vmin=vminIn, vmax=vmaxIn)
  ax.get_xaxis().set_visible(False)
  ax.get_yaxis().set_visible(False)
  ax.set_xlim([0,1])
  ax.set_ylim([0,1])
  ax.set_aspect(aspect=1)


def doFigure(figId, c0rs, c1rs, c2rs, xrs, yrs, bd):
  fig = plt.figure(1)
  ax1 = fig.add_subplot(131)
  ax2 = fig.add_subplot(132)
  ax3 = fig.add_subplot(133)

  plotScatter(xrs, yrs, c0rs, ax1, "$c_0$",
              cm.jet, bd['c0'][0], bd['c0'][1])
  plotScatter(xrs, yrs, c1rs, ax2, "$c_1$",
              cm.brg, bd['c1'][0], bd['c1'][1])
  plotScatter(xrs, yrs, c2rs, ax3, "$c_2$",
              cm.terrain, bd['c2'][0], bd['c2'][1])


def plotSingleSnapshot(n, xrs, yrs, snapId, snaps, samplingFreq, dt):
  # get only target state from snapshot matrix
  targetState = snaps[:, snapId]

  # split state by species
  [c0,c1,c2,c0rs,c1rs,c2rs] = splitStateBySpecies(targetState, n)

  # calculate the time this snapshot corresponds to
  snapshotTime = computeTimeOfSnapshot(snapId, samplingFreq, dt)

  boundsDic = computeBounds(c0, c1, c2)
  printBounds(boundsDic)

  doFigure(1, c0rs, c1rs, c2rs, xrs, yrs, boundsDic)
  plt.show()


def plotSequence(n, xrs, yrs, fromSnapId, toSnapId, snaps, samplingFreq, dt):
  for i in range(fromSnapId, toSnapId+1):
    # get target state from snapshot matrix
    targetState = snaps[:, i]

    # split state by species
    [c0,c1,c2,c0rs,c1rs,c2rs] = splitStateBySpecies(targetState, n)

    # calculate the time this snapshot corresponds to
    snapshotTime = computeTimeOfSnapshot(i, samplingFreq, dt)

    boundsDic = computeBounds(c0, c1, c2)
    printBounds(boundsDic)
    doFigure(1, c0rs, c1rs, c2rs, xrs, yrs, boundsDic)
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
