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

def computeTimingsStat(data, statType):
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


#=====================================================================
#
#  do bar plot with side by side bars
#
#=====================================================================
def plotBarRegular(cppDic, pyDic, meshLabels, romSizes, romSizesStr):
  if len(romSizes) != 3:
    raise Exception('The style for the bar plot currently support 3 rom sizes only')

  # Setting the positions and width for the bars
  pos = list(range(len(meshLabels)))
  width = 0.12 # the width of a bar

  fig, ax = plt.subplots(figsize=(10,6))
  plt.grid()
  ax.set_axisbelow(True)

  # rom1
  romSize1 = romSizesStr[0]
  leg = 'c++, n=' + str(romSize1)
  bar1=plt.bar([p-width*1.15 for p in pos], cppDic[romSize1], width,
               alpha=0.5, color='r', hatch='xxx', edgecolor='k', label=leg)
  leg = 'py, n=' + str(romSize1)
  plt.bar([p-width*0.15 for p in pos], pyDic[romSize1], width, alpha=0.5,
          color='r', hatch='/////', edgecolor='k', label=leg)

  # rom size 2
  romSize2 = romSizesStr[1]
  leg = 'c++, n=' + str(romSize2)
  plt.bar([p+width for p in pos], cppDic[romSize2], width,
          alpha=0.5, color='b', hatch='xxx', edgecolor='k',label=leg)
  leg = 'py, n=' + str(romSize2)
  plt.bar([p+width*2 for p in pos], pyDic[romSize2], width,
          alpha=0.5, color='b', hatch='/////', edgecolor='k',label=leg)

  # rom size 3
  romSize3 = romSizesStr[2]
  leg = 'c++, n=' + str(romSize3)
  plt.bar([p+width*3.15 for p in pos], cppDic[romSize3], width,
          alpha=0.5, color='g', hatch='xxx', edgecolor='k',label=leg)
  leg = 'py, n=' + str(romSize3)
  plt.bar([p+width*4.15 for p in pos], pyDic[romSize3], width,
          alpha=0.5, color='g', hatch='/////', edgecolor='k',label=leg)

  # Setting axis labels and ticks
  ax.set_ylabel('Overhead wrt fastest C++ run')
  ax.set_xlabel('Mesh Size')
  # ticks for groups
  ax.set_xticks([p + width*1.5 for p in pos])
  ax.set_xticklabels(meshLabels)
  # Setting the x-axis and y-axis limits
  plt.xlim(min(pos)-width*3, max(pos)+width*6)
  #plt.ylim([0, max(green_data + blue_data + red_data) * 1.5])
  plt.legend(loc='upper left')


#=====================================================================
# plot overhead above the bars
#=====================================================================
def plotTextOverhead(xLoc, ovhead, cppVal, diffVal, maxY):
  for i in range(len(xLoc)):
    string = '{:3.0f}'.format(ovhead[i])
    thisX, thisY = xLoc[i], np.maximum(cppVal[i]+diffVal[i], cppVal[i])
    plt.text(x=thisX, y=thisY+0.065*maxY, s=string+'%',
             size = 8,
             rotation=90,
             horizontalalignment='center',
             verticalalignment='center')

#=====================================================================
# plot one set of bars for cpp and python
# this means that for both cpp and python and a given romsize,
# we plot the bars at each meshsize
#=====================================================================
def plotBarSet(xLoc, width, romSize, cppDic, pyDic, barColors, hatches, maxY):
  cppVal = cppDic[romSize]
  diffVal= np.asarray(pyDic[romSize]) - np.asarray(cppVal)
  # compute overhead in % wrt cpp case
  ovhead = np.abs(diffVal)/np.asarray(cppVal)*100
  # display text for overhead as percent on top of bar
  plotTextOverhead(xLoc, ovhead, cppVal, diffVal, maxY)

  # plot bar cpp
  leg = 'c++, n=' + str(romSize)
  plt.bar(xLoc, cppVal, width, alpha=0.5, color=barColors['cpp'], hatch=hatches['cpp'], edgecolor='k')
  # plot bar py
  leg = 'py, n=' + str(romSize)
  plt.bar(xLoc, diffVal, width, bottom=cppVal, alpha=0.5, color=barColors['py'], hatch=hatches['py'], edgecolor='k')


#=====================================================================
#
#  do bar plot with stacking to show the differences
#
#=====================================================================
def plotBarStacked(cppDic, pyDic, meshLabels, romSizes, romSizesStr):
  # if len(romSizes) != 3:
  #   raise Exception('The style for the bar plot currently support 3 rom sizes only')

  # number of mesh sizes to deal with
  numMeshes = len(meshLabels)

  # Setting the positions and width for the bars
  posArray = range(numMeshes)
  pos = list(posArray)
  width = 0.125 # the width of a bar

  fig, ax = plt.subplots(figsize=(8,6))
  plt.grid()
  ax2 = ax.twiny()
  fig.subplots_adjust(bottom=0.25)

  colors = {'cpp': 'w', 'py': 'w'}
  hatches = {'cpp': '////', 'py':'xxx'}

  # compute the value of the heighest bar
  maxY = 0.
  for it in range(len(romSizes)):
    currRomSize = romSizesStr[it]
    maxYcpp, maxYpy = np.max(cppDic[currRomSize]), np.max(pyDic[currRomSize])
    maxY = np.max([maxY, maxYcpp, maxYpy])

  #---- loop over rom sizes and plot ----
  xTicksBars, xTlabels = [], []
  for it in range(len(romSizes)):
    # x locations for the bars
    shift = width*it
    xLoc = [p+shift for p in pos]
    currRomSize = romSizesStr[it]
    plotBarSet(xLoc, width, currRomSize, cppDic, pyDic, colors, hatches, maxY)
    xTicksBars += [p+shift for p in pos]
    xTlabels += [romSizesStr[it] for i in range(numMeshes)]


  # remove the vertical lines of the grid
  ax.xaxis.grid(which="major", color='None', linestyle='-.', linewidth=0)

  ax.xaxis.set_ticks_position('bottom')
  ax.xaxis.set_label_position('bottom')
  ax.set_xticks(xTicksBars)
  ax.set_xticklabels(xTlabels)
  ax.xaxis.set_tick_params(rotation=90)
  ax.set_xlabel('Rom Sizes')

  # ticks for the meshes
  meshTicks = posArray+1.95*width*np.ones(numMeshes)
  ax2.set_xticks(meshTicks)
  ax2.set_xticklabels(meshLabels)
  ax2.xaxis.set_ticks_position('bottom')
  ax2.xaxis.set_label_position('bottom')
  ax2.spines['bottom'].set_position(('outward', 50))
  ax2.set_xlabel('Mesh Sizes')

  # plt.text(x=-0.175, y=-0.3, s='Mesh Size',size = 10,rotation=0,
  #          horizontalalignment='center',verticalalignment='center')
  #plt.yscale('log')
  ax.set_xlim(min(pos)-width*2, max(pos)+width*5)
  ax2.set_xlim(min(pos)-width*2, max(pos)+width*5)
  plt.ylim([0, maxY*1.1])
  #plt.legend(loc='upper left')







def main(cppDataDir, pyDataDir, romName, barType, statType):
  # check that target directories exist
  if not os.path.isdir(cppDataDir):
    raise Exception('The data dir {} does not exist. '.format(cppDataDir))
  if not os.path.isdir(pyDataDir):
    raise Exception('The data dir {} does not exist. '.format(pyDataDir))

  # the name of the subfolder where data lives
  dataSubDir = "data_" + romName
  # the file name (it is same not matter if cpp or python)
  cppFileName = "burgers1d_rom_" + romName + "_timings.txt"
  pyFileName  = "main_rom_" + romName + "_timings.txt"

  # check that data folders have the data in them
  cppDataFilePath = cppDataDir + "/" + dataSubDir + "/" + cppFileName
  pyDataFilePath  = pyDataDir  + "/" + dataSubDir + "/" + pyFileName
  if not os.path.isfile(cppDataFilePath):
    raise Exception('The file {} does not exist. '.format(cppDataFilePath))
  if not os.path.isfile(pyDataFilePath):
    raise Exception('The file {} does not exist. '.format(pyDataFilePath))

  # load data
  cppData, pyData = np.loadtxt(cppDataFilePath), np.loadtxt(pyDataFilePath)

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

  # compute the avg timings
  cppDataAvg, pyDataAvg = computeTimingsStat(cppData, statType), computeTimingsStat(pyData, statType)
  print(cppDataAvg)
  print(pyDataAvg)

  # create dictionary for bar plotting
  cppDic, pyDic = createDicByRomSize(cppDataAvg), createDicByRomSize(pyDataAvg)

  # do regular or stacked plot
  if barType == "default":
    plotBarRegular(cppDic, pyDic, meshLabels, romSizes, romSizesStr)
  elif barType == "stacked":
    plotBarStacked(cppDic, pyDic, meshLabels, romSizes, romSizesStr)
  else:
    raise Exception('Invalid choice for bar type {} '.format(barType))
  plt.show()



#////////////////////////////////////////////
if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-cpp-data-dir", "--cpp-data-dir", dest="cppDataDir")
  parser.add_argument("-py-data-dir",  "--py-data-dir", dest="pyDataDir")
  parser.add_argument("-method",       "--method", dest="romName",
                      help = "The ROM method you are trying to plot: lspg or galerkin" )
  parser.add_argument("-bar-type",     "--bar-type", dest="barType",
                      help = "Which bar plot type to plot, stacked or default.")
  parser.add_argument("-stat-type",    "--stat-type", dest="statType",
                      help = "Which statistic to compute from data: \
                              mean, gmean (geometric mean), q50 (50th percentile).")
  args = parser.parse_args()
  main(args.cppDataDir, args.pyDataDir, args.romName, args.barType, args.statType)
#////////////////////////////////////////////
