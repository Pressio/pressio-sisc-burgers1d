#!/usr/bin/env python

import sys, os, time
import subprocess
import numpy as np
import os.path, re
from argparse import ArgumentParser
import matplotlib.pyplot as plt

np.set_printoptions(edgeitems=10, linewidth=100000)

def extractMeshSizes(data):
  # the mesh sizes should be in the first col of the data
  return np.unique(data[:, 0])

def extractRomSizes(data):
  # the rom sizes should be in the second col of the data
  return np.unique(data[:, 1])

def computeMeanTimings(data):
  nRows, nCols = data.shape[0], data.shape[1]
  # multiple time values are stored from col 2 to ...
  # compute the mean along each row
  # we store
  res = np.zeros((nRows, 3)) #3 cols, we store mesh size, rom size, and mean time
  res[:,0:2] = data[:, 0:2]
  res[:, 2]  = np.mean(data[:, 2:], axis=1)
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


def plotBarRegular(cppDic, pyDic, meshLabels, romSizes, romSizesStr):
  #-----------------------------
  # Plotting the bars side by side
  #-----------------------------
  # Setting the positions and width for the bars
  pos = list(range(len(meshLabels)))
  width = 0.12 # the width of a bar

  fig, ax = plt.subplots(figsize=(10,6))
  plt.grid()
  ax.set_axisbelow(True)

  if len(romSizes) != 3:
    raise Exception('The style for the bar plot currently support 3 rom sizes only')

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

  # ticks for rom size1
  #ax.set_xticks([p + 1.5 * width for p in pos])
  #ax.set_xticklabels(meshLabels)
  #ax.set_yticks([i for i in range(0, 60, 2)])

  # Setting the x-axis and y-axis limits
  plt.xlim(min(pos)-width*3, max(pos)+width*6)
  #plt.ylim([0, max(green_data + blue_data + red_data) * 1.5])
  plt.legend(loc='upper left')


def plotTextOverhead(xLoc, ovhead, cppVal, diffVal):
  for i in range(len(xLoc)):
    string = '{:3.0f}'.format(ovhead[i])
    thisX, thisY = xLoc[i], cppVal[i]+diffVal[i]
    plt.text(x=thisX, y=thisY+0.15, s=string+'%',
             size = 8,
             rotation=90,
             horizontalalignment='center',
             verticalalignment='center')

def plotBarSet(xLoc, width, romSize, cppDic, pyDic, barColors, hatches):
  cppVal = cppDic[romSize]
  diffVal= np.asarray(pyDic[romSize]) - np.asarray(cppVal)
  # compute overhead in % wrt cpp case
  ovhead = np.abs(diffVal)/np.asarray(cppVal)*100
  # display text for overhead as percent on top of bar
  plotTextOverhead(xLoc, ovhead, cppVal, diffVal)
  # plot bar cpp
  leg = 'c++, n=' + str(romSize)
  plt.bar(xLoc, cppVal, width, alpha=0.5, color=barColors['cpp'], hatch=hatches['cpp'], edgecolor='k')
  # plot bar py
  leg = 'py, n=' + str(romSize)
  plt.bar(xLoc, diffVal, width, bottom=cppVal, alpha=0.5, color=barColors['py'], hatch=hatches['py'], edgecolor='k')


def plotBarStacked(cppDic, pyDic, meshLabels, romSizes, romSizesStr):
  # Setting the positions and width for the bars
  pos = list(range(len(meshLabels)))
  width = 0.12 # the width of a bar

  fig, ax = plt.subplots(figsize=(10,6))
  plt.grid()
  ax.set_axisbelow(True)

  if len(romSizes) != 3:
    raise Exception('The style for the bar plot currently support 3 rom sizes only')

  # since we use stacked bar, we plot cpp and stack on top the difference
  colors = {'cpp': 'k', 'py': 'r'}
  hatches = {'cpp': 'xxx', 'py':'/////'}

  #---- first rom size -----
  # x locations for the bars
  xLoc = [p-width*0.15 for p in pos]
  plotBarSet(xLoc, width, romSizesStr[0], cppDic, pyDic, colors, hatches)

  #---- second rom size ----
  xLoc = [p+width for p in pos]
  plotBarSet(xLoc, width, romSizesStr[1], cppDic, pyDic, colors, hatches)

  #---- second rom size ----
  xLoc = [p+width*2.15 for p in pos]
  plotBarSet(xLoc, width, romSizesStr[2], cppDic, pyDic, colors, hatches)

  #ax.set_ylabel('... bla bla')
  ax.set_xlabel('Mesh Size')
  # ticks for groups
  ax.set_xticks([p + width*1 for p in pos])
  ax.set_xticklabels(meshLabels)

  # ticks for rom size1
  #ax.set_xticks([p + 1.5 * width for p in pos])
  #ax.set_xticklabels(meshLabels)
  #ax.set_yticks([i for i in range(0, 60, 2)])

  # Setting the x-axis and y-axis limits
  plt.xlim(min(pos)-width*3, max(pos)+width*6)
  #plt.ylim([0, max(green_data + blue_data + red_data) * 1.5])
  #plt.legend(loc='upper left')







def main(cppDataDir, pyDataDir, romName, barType):
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

  # compute the avg timings
  cppDataAvg, pyDataAvg = computeMeanTimings(cppData), computeMeanTimings(pyData)
  print(cppDataAvg)
  print(pyDataAvg)

  # # rescale data by the min value
  # minCppTime = np.min(cppDataAvg[:, 2:])
  # cppDataAvg[:, 2:] /= minCppTime
  # pyDataAvg[:, 2:] /= minCppTime

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
  parser.add_argument("-py-data-dir", "--py-data-dir", dest="pyDataDir")
  parser.add_argument("-method", "--method", dest="romName")
  parser.add_argument("-bar-type", "--bar-type", dest="barType")
  args = parser.parse_args()
  main(args.cppDataDir, args.pyDataDir, args.romName, args.barType)
#////////////////////////////////////////////
