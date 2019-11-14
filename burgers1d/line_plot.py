#!/usr/bin/env python

import sys, os, time
import subprocess, math
import numpy as np
import os.path, re
from scipy import stats
from argparse import ArgumentParser
import matplotlib.pyplot as plt

np.set_printoptions(edgeitems=10, linewidth=100000)

def extractMeshSizes(data):
  # the mesh sizes should be in the first col of the data
  return np.unique(data[:, 0])

def extractRomSizes(data):
  # the rom sizes should be in the second col of the data
  return np.unique(data[:, 1])

def computeTimingsStat(data, label, statType):
  nRows, nCols = data.shape[0], data.shape[1]
  # multiple time values are stored from col 2 to ...
  # compute the mean along each row
  # we store
  res = np.zeros((nRows, 3)) #3 cols, we store mesh size, rom size, and mean time
  res[:,0:2] = data[:, 0:2]
  if statType == "mean":
    res[:, 2] = np.mean(data[:, 2:], axis=1)
  elif statType == "gmean":
    res[:, 2] = stats.mstats.gmean(data[:, 2:], axis=1)
  elif statType == "q50":
    res[:, 2] = np.percentile(data[:, 2:], 50, axis=1)
  elif statType == "worst":
    # take min of cpp and max of python to show the worst case scenario
    if label == "c++":
      res[:, 2] = np.min(data[:, 2:], axis=1)
    else:
      res[:, 2] = np.max(data[:, 2:], axis=1)
  elif statType == "best":
    # take max of cpp and min of python to show the worst case scenario
    if label == "c++":
      res[:, 2] = np.max(data[:, 2:], axis=1)
    else:
      res[:, 2] = np.min(data[:, 2:], axis=1)

  else:
    raise Exception('Invalid choice for the statType = {}'.format(statType))

  return res

def createDicByRomSize(data):
  # create a dictionary where
  # key = rom size
  # value = array with timings at each mesh size
  dic = {}
  nr = data.shape[0]
  for i in range(nr):
    thisRomSize = str(int(data[i][1]))
    if thisRomSize in dic:
      dic[thisRomSize].append(data[i][2])
    else:
      dic[thisRomSize] = [data[i][2]]
  return dic


def doPlot(cppDic, pyDic, meshSizes, meshLabels, romSizes, romSizesStr):
  # number of mesh sizes to deal with
  numMeshes = len(meshLabels)

  fig, ax = plt.subplots() #figsize=(8,6))
  plt.grid()

  colors = {'cpp': 'k', 'py': 'r'}
  hatches = {'cpp': '////', 'py':'xxx'}

  #---- loop over rom sizes and plot ----
  for it in range(len(romSizes)):
    currRomSize = romSizesStr[it]
    cppData, pyData = cppDic[currRomSize], pyDic[currRomSize]
    plt.plot(meshSizes, cppData, '-o', color=colors['cpp'], linewidth=2)
    plt.plot(meshSizes, pyData,  '-s', color=colors['py'], linewidth=2)

  # # remove the vertical lines of the grid
  # ax.xaxis.grid(which="major", color='None', linestyle='-.', linewidth=0)

  # ax.xaxis.set_ticks_position('bottom')
  # ax.xaxis.set_label_position('bottom')
  # ax.set_xticks(xTicksBars)
  # ax.set_xticklabels(xTlabels)
  # ax.xaxis.set_tick_params(rotation=90)
  # ax.set_xlabel('Rom Sizes')

  # # ticks for the meshes
  # meshTicks = posArray+1.95*width*np.ones(numMeshes)
  # ax2.set_xticks(meshTicks)
  # ax2.set_xticklabels(meshLabels)
  # ax2.xaxis.set_ticks_position('bottom')
  # ax2.xaxis.set_label_position('bottom')
  # ax2.spines['bottom'].set_position(('outward', 50))
  # ax2.set_xlabel('Mesh Sizes')

  # plt.text(x=-0.175, y=-0.3, s='Mesh Size',size = 10,rotation=0,
  #          horizontalalignment='center',verticalalignment='center')
  plt.xscale('log')
  plt.yscale('log')
  # ax.set_xlim(min(pos)-width*2, max(pos)+width*5)
  # ax2.set_xlim(min(pos)-width*2, max(pos)+width*5)
  # plt.ylim([0, maxY*1.1])
  #plt.legend(loc='upper left')


#=====================================================================
#   main
#=====================================================================
def main(cppFile, pyFile, romName, statType):
  # check that files exists
  if not os.path.isfile(cppFile):
    raise Exception('The file {} does not exist. '.format(cppFile))
  if not os.path.isfile(pyFile):
    raise Exception('The file {} does not exist. '.format(pyFile))

  # load data
  cppData, pyData = np.loadtxt(cppFile), np.loadtxt(pyFile)

  # extract mesh sizes from cpp and python
  cppMeshSizes, pyMeshSizes = extractMeshSizes(cppData), extractMeshSizes(pyData)
  # check cpp and py data have same mesh sizes
  assert( np.array_equal(cppMeshSizes, pyMeshSizes))
  # since cpp/py mesh sizes are same, does not matter which one we use
  meshSizes = cppMeshSizes
  # use the mesh sizes from
  meshLabels = [str(int(i)) for i in meshSizes]

  # extract rom sizes from cpp and python
  cppRomSizes, pyRomSizes = extractRomSizes(cppData), extractRomSizes(pyData)
  # check cpp and py data have same roms sizes
  assert( np.array_equal(cppRomSizes, pyRomSizes))
  # since cpp/py rom sizes are same, does not matter which one we use
  romSizes = cppRomSizes
  romSizesStr = [str(int(i)) for i in romSizes]
  print(romSizesStr)

  # compute the timings stat
  cppDataAvg = computeTimingsStat(cppData, "c++", statType)
  pyDataAvg  = computeTimingsStat(pyData, "py", statType)
  print(cppDataAvg)
  print(pyDataAvg)

  # create dictionary for plotting
  cppDic, pyDic = createDicByRomSize(cppDataAvg), createDicByRomSize(pyDataAvg)

  #do regular or stacked plot
  doPlot(cppDic, pyDic, meshSizes, meshLabels, romSizes, romSizesStr)
  plt.show()

#////////////////////////////////////////////
if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-cpp-file", "--cpp-file", dest="cppFile")
  parser.add_argument("-py-file",  "--py-file", dest="pyFile")
  parser.add_argument("-method",       "--method", dest="romName",
                      help = "The ROM method you are trying to plot: lspg or galerkin" )
  parser.add_argument("-stat-type",    "--stat-type", dest="statType",
                      help = "Which statistic to compute from data: \
                              mean, gmean (geometric mean), q50 (50th percentile), \
                              worst (min of c++ and max of Python to show worst case),\
                              best (max of c++ and min of Python to show best case)")
  args = parser.parse_args()
  main(args.cppFile, args.pyFile, args.romName, args.statType)
#////////////////////////////////////////////
