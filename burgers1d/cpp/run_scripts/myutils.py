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
  os.system("sed -i '' s/numCellValue/" + str(numCell) + "/g input.txt")
  os.system("sed -i '' s/dtValue/" + str(constants.dt) + "/g input.txt")
  os.system("sed -i '' s/finalTimeValue/" + str(constants.finalTime) + "/g input.txt")
  os.system("sed -i '' s/observerOnValue/" + str(0) + "/g input.txt")
  os.system("sed -i '' s/romOnValue/" + str(0) + "/g input.txt")


def createInputFileFomForBasis(numCell, samplingFreq):
  # copy template
  os.system('cp input.template input.txt')
  os.system("sed -i '' s/numCellValue/" + str(numCell) + "/g input.txt")
  os.system("sed -i '' s/dtValue/" + str(constants.dt) + "/g input.txt")
  os.system("sed -i '' s/finalTimeValue/" + str(constants.finalTime) + "/g input.txt")
  os.system("sed -i '' s/observerOnValue/" + str(1) + "/g input.txt")
  os.system("sed -i '' s/shapshotsFreqValue/" + str(samplingFreq) + "/g input.txt")
  os.system("sed -i '' s/shapshotsFileNameValue/snapshots.txt/g input.txt")
  os.system("sed -i '' s/basisFileNameValue/basis.txt/g input.txt")
  os.system("sed -i '' s/romOnValue/" + str(0) + "/g input.txt")


def createInputFileRom(numCell, romSize):
  # copy template
  os.system('cp input.template input.txt')
  os.system("sed -i '' s/numCellValue/" + str(numCell) + "/g input.txt")
  os.system("sed -i '' s/dtValue/" + str(constants.dt) + "/g input.txt")
  os.system("sed -i '' s/finalTimeValue/" + str(constants.finalTime) + "/g input.txt")

  os.system("sed -i '' s/observerOnValue/" + str(0) + "/g input.txt")
  os.system("sed -i '' s/basisFileNameValue/basis.txt/g input.txt")
  os.system("sed -i '' s/romOnValue/" + str(1) + "/g input.txt")
  os.system("sed -i '' s/romSizeValue/" + str(romSize) + "/g input.txt")
