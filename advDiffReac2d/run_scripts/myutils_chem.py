#!/usr/bin/env python

import sys, os, time, re
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
import os.path

import constants_chem as cch

# store current directory
pwd = os.getcwd()

def createInputFileFomChemTiming(meshFileName):
  # copy template
  os.system('cp input.template input.txt')
  # using @ helps when string contains slashes
  os.system("sed -i '' s@meshFileNameValue@"  + meshFileName + "@g input.txt")
  os.system("sed -i '' s@dtValue@"            + str(cch.dt) + "@g input.txt")
  os.system("sed -i '' s@finalTimeValue@"     + str(cch.finalTime) + "@g input.txt")

  os.system("sed -i '' s@diffusionValue@"     + str(cch.diffusion) + "@g input.txt")
  os.system("sed -i '' s@chemReactionValue@"  + str(cch.chemReaction) + "@g input.txt")

  os.system("sed -i '' s@observerOnValue@"    + str(0) + "@g input.txt")
  os.system("sed -i '' s@romOnValue@"    + str(0) + "@g input.txt")



def createInputFileFomChemForBasis(meshFileName, samplingFreq):
  # copy template
  os.system('cp input.template input.txt')
  # using @ helps when string contains slashes
  os.system("sed -i '' s@meshFileNameValue@"  + meshFileName + "@g input.txt")
  os.system("sed -i '' s@dtValue@"            + str(cch.dt) + "@g input.txt")
  os.system("sed -i '' s@finalTimeValue@"     + str(cch.finalTime) + "@g input.txt")

  os.system("sed -i '' s@diffusionValue@"     + str(cch.diffusion) + "@g input.txt")
  os.system("sed -i '' s@chemReactionValue@"  + str(cch.chemReaction) + "@g input.txt")

  os.system("sed -i '' s@observerOnValue@"    + str(1) + "@g input.txt")
  os.system("sed -i '' s@shapshotsFreqValue@" + str(samplingFreq) + "@g input.txt")
  os.system("sed -i '' s@shapshotsFileNameValue@snapshots.txt@g input.txt")
  os.system("sed -i '' s@basisFileNameValue@basis.txt@g input.txt")

  os.system("sed -i '' s@romOnValue@"    + str(0) + "@g input.txt")
