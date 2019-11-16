#!/usr/bin/env python

import sys, os, time, re
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
import os.path

import constants

# store current directory
pwd = os.getcwd()

def createInputFileFomTiming(meshSize):
  # copy template
  os.system('cp input.template input.txt')

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("numCellValue",   str(meshSize) )
  data = data.replace("dtValue",        str(constants.dt) )
  data = data.replace("finalTimeValue", str(constants.finalTimeTiming[meshSize]) )
  data = data.replace("observerOnValue",str(0) )
  data = data.replace("romOnValue",     str(0) )
  fin.close()

  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()


def createInputFileFomForBasis(meshSize, samplingFreq):
  createInputFileFomTiming(meshSize)

  # copy template
  os.system('cp input.template input.txt')

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("numCellValue",           str(meshSize) )
  data = data.replace("dtValue",                str(constants.dt) )
  data = data.replace("finalTimeValue",         str(constants.finalTimeBasis) )
  data = data.replace("observerOnValue",        str(1) )
  data = data.replace("romOnValue",             str(0) )
  data = data.replace("shapshotsFreqValue",     str(samplingFreq) )
  data = data.replace("shapshotsFileNameValue", "snapshots.txt" )
  data = data.replace("basisFileNameValue",     "basis.txt")
  fin.close()

  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()


def createInputFileRom(meshSize, romSize):
  # copy template
  os.system('cp input.template input.txt')

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("numCellValue",           str(meshSize) )
  data = data.replace("dtValue",                str(constants.dt) )
  data = data.replace("finalTimeValue",         str(constants.finalTimeTiming[meshSize]) )
  data = data.replace("observerOnValue",        str(0) )
  data = data.replace("basisFileNameValue",     "basis.txt")
  data = data.replace("romOnValue",             str(1) )
  data = data.replace("romSizeValue",           str(romSize) )
  fin.close()

  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()
