#!/usr/bin/env python

import sys, os, time, re
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
import os.path

import constants

# store current directory
pwd = os.getcwd()

def createInputFileFomTiming(numCell):
  # copy template
  os.system('cp input.template input.txt')

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("numCellValue",   str(numCell) )
  data = data.replace("dtValue",        str(constants.dt) )
  data = data.replace("finalTimeValue", str(constants.finalTime) )
  data = data.replace("observerOnValue",str(0) )
  data = data.replace("romOnValue",     str(0) )
  fin.close()

  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()


def createInputFileFomForBasis(numCell, samplingFreq):
  createInputFileFomTiming(numCell)

  # copy template
  os.system('cp input.template input.txt')

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("numCellValue",           str(numCell) )
  data = data.replace("dtValue",                str(constants.dt) )
  data = data.replace("finalTimeValue",         str(constants.finalTime) )
  data = data.replace("observerOnValue",        str(1) )
  data = data.replace("romOnValue",             str(0) )
  data = data.replace("shapshotsFreqValue",     str(samplingFreq) )
  data = data.replace("shapshotsFileNameValue", "snapshots.txt" )
  data = data.replace("basisFileNameValue",     "basis.txt")
  fin.close()

  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()


def createInputFileRom(numCell, romSize):
  # copy template
  os.system('cp input.template input.txt')

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("numCellValue",           str(numCell) )
  data = data.replace("dtValue",                str(constants.dt) )
  data = data.replace("finalTimeValue",         str(constants.finalTime) )
  data = data.replace("observerOnValue",        str(0) )
  data = data.replace("basisFileNameValue",     "basis.txt")
  data = data.replace("romOnValue",             str(1) )
  data = data.replace("romSizeValue",           str(romSize) )
  fin.close()

  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()
