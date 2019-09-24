#!/usr/bin/env python

import sys, os, time, re
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
import os.path

import constants_ms as cms

def createInputFileFomEigenMs(stepperName, meshFileName):
  # copy template
  os.system('cp input.template input.txt')

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("problemNameValue",       "ms")
  data = data.replace("meshFileNameValue",      meshFileName)
  data = data.replace("odeStepperNameValue",    stepperName)
  data = data.replace("dtValue",                str(cms.dt) )
  data = data.replace("finalTimeValue",         str(cms.finalTime) )
  data = data.replace("diffusionValue",         str(cms.diffusion) )
  data = data.replace("chemReactionValue",      str(cms.chemReaction) )
  data = data.replace("observerOnValue",        str(1) )
  data = data.replace("shapshotsFreqValue",     str(cms.samplingFreq) )
  data = data.replace("shapshotsFileNameValue", "snapshots.txt")
  data = data.replace("romOnValue",             str(0) )
  fin.close()

  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()

  # # # using @ helps when string contains slashes
  # os.system("sed -i '' s/problemNameValue/ms/g input.txt")
  # os.system("sed -i '' s/meshFileNameValue/"  + meshFileName            + "/g input.txt")

  # os.system("sed -i '' s/odeStepperNameValue/"+ stepperName             + "/g input.txt")
  # os.system("sed -i '' s/dtValue/"            + str(cms.dt)             + "/g input.txt")
  # os.system("sed -i '' s/finalTimeValue/"     + str(cms.finalTime)      + "/g input.txt")

  # os.system("sed -i '' s/diffusionValue/"     + str(cms.diffusion)      + "/g input.txt")
  # os.system("sed -i '' s/chemReactionValue/"  + str(cms.chemReaction)   + "/g input.txt")

  # os.system("sed -i '' s/observerOnValue/"    + str(1)                  + "/g input.txt")
  # os.system("sed -i '' s/shapshotsFreqValue/" + str(cms.samplingFreq)   + "/g input.txt")
  # os.system("sed -i '' s/shapshotsFileNameValue/snapshots.txt/g input.txt")
  # os.system("sed -i '' s/romOnValue/"         + str(0)                  + "/g input.txt")
