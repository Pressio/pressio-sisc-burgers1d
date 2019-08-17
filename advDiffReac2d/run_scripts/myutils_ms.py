#!/usr/bin/env python

import sys, os, time, re
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
import os.path

import constants_ms as cms

def createInputFileFomEigenMs(stepperName, meshFileName):
  # copy template
  os.system('cp input.template input.txt')

  # using @ helps when string contains slashes
  os.system("sed -i '' s@problemNameValue@ms@g input.txt")
  os.system("sed -i '' s@meshFileNameValue@"  + meshFileName            + "@g input.txt")

  os.system("sed -i '' s@odeStepperNameValue@"+ stepperName             + "@g input.txt")
  os.system("sed -i '' s@dtValue@"            + str(cms.dt)             + "@g input.txt")
  os.system("sed -i '' s@finalTimeValue@"     + str(cms.finalTime)      + "@g input.txt")

  os.system("sed -i '' s@diffusionValue@"     + str(cms.diffusion)      + "@g input.txt")
  os.system("sed -i '' s@chemReactionValue@"  + str(cms.chemReaction)   + "@g input.txt")

  os.system("sed -i '' s@observerOnValue@"    + str(1)                  + "@g input.txt")
  os.system("sed -i '' s@shapshotsFreqValue@" + str(cms.samplingFreq)   + "@g input.txt")
  os.system("sed -i '' s@shapshotsFileNameValue@snapshots.txt@g input.txt")
  os.system("sed -i '' s@romOnValue@"         + str(0)                  + "@g input.txt")
