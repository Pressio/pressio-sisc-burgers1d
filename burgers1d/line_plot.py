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


def doPlot(cppDic, pyDic, meshSizes, meshLabels, romSizes, romSizesStr, fileName):
  # number of mesh sizes to deal with
  numMeshes = len(meshLabels)

  fig, ax = plt.subplots() #figsize=(8,6))
  plt.grid()
  colors = {'cpp': 'k', 'py': 'r'}
  mark = { 0:'o', 1:'s', 2:'x', 3:'^'}
  ms = 7
  lw = 1.5

  #---- loop over rom sizes and plot ----
  for it in range(len(romSizes)):
    currRomSize = romSizesStr[it]
    cppData, pyData = cppDic[currRomSize], pyDic[currRomSize]
    # convert from seconds to ms
    cppData, pyData = np.asarray(cppData)*1000, np.asarray(pyData)*1000
    plt.plot(meshSizes, cppData, '-', marker=mark[it], markersize=ms,
             markerfacecolor='none', color=colors['cpp'], linewidth=lw)
    plt.plot(meshSizes, pyData,  '-', marker=mark[it], markersize=ms,
             markerfacecolor='none', color=colors['py'], linewidth=lw)

  plt.xscale('log')
  plt.yscale('log')
  plt.minorticks_off()
  # # remove the vertical lines of the grid
  # ax.xaxis.grid(which="major", color='None', linestyle='-.', linewidth=0)
  ax.xaxis.set_ticks_position('bottom')
  ax.xaxis.set_label_position('bottom')
  ax.set_xticks(meshSizes)
  ax.set_xticklabels(meshLabels)
  ax.set_xlabel('Mesh Size', fontsize=15)

  ax.set_ylabel('Time per iteration (ms)', fontsize=15)
  plt.xticks(fontsize=14)
  plt.yticks(fontsize=14)

  # dummy things for the legend
  plt.plot([10, 10], [-10, -10], '-', color=colors['cpp'], label='pressio')
  plt.plot([10, 10], [-10, -10], '-', color=colors['py'], label='pressio4py')
  nrs  = len(romSizes)
  for i in range(nrs):
    plt.plot([10, 10], [-10, -10], '-', marker=mark[nrs-i-1], color='k', markerfacecolor='none',
             linewidth=0, label='n='+romSizesStr[nrs-i-1])

  ax.set_xlim(600, 50000)#min(meshSizes)-128, max(meshSizes)+512)
  plt.ylim([1e-2, 1000000])
  plt.legend(loc='upper left', fontsize=11)
  fig.savefig(fileName+".pdf", bbox_inches='tight', dpi=600)


#=====================================================================
#   main
#=====================================================================
def main(cppFile, pyFile, romName, statType, fileName):
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
  doPlot(cppDic, pyDic, meshSizes, meshLabels, romSizes, romSizesStr, fileName)
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
  parser.add_argument("-filename",       "--filename", dest="fileName",
                      help = "The file name to print figure")
  args = parser.parse_args()
  main(args.cppFile, args.pyFile, args.romName, args.statType, args.fileName)
#////////////////////////////////////////////
