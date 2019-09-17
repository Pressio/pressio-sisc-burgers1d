#!/usr/bin/env python

import sys, os, time, re
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
import os.path

import constants_chem as cch

# TODO: just reuse code here, no need to repeat all commands all the time


def createInputFileFomChemTiming(stepperName, meshFileName):
  # copy template
  os.system('cp input.template input.txt')

  os.system("sed -i '' s@problemNameValue@chemABC@g input.txt")
  os.system("sed -i '' s@meshFileNameValue@"  + meshFileName            + "@g input.txt")

  os.system("sed -i '' s@odeStepperNameValue@"+ stepperName             + "@g input.txt")
  os.system("sed -i '' s@dtValue@"            + str(cch.dt)             + "@g input.txt")
  os.system("sed -i '' s@finalTimeValue@"     + str(cch.finalTime)      + "@g input.txt")

  os.system("sed -i '' s@diffusionValue@"     + str(cch.diffusion)      + "@g input.txt")
  os.system("sed -i '' s@chemReactionValue@"  + str(cch.chemReaction)   + "@g input.txt")

  os.system("sed -i '' s@observerOnValue@"    + str(0)                  + "@g input.txt")
  os.system("sed -i '' s@romOnValue@"         + str(0)                  + "@g input.txt")


def createInputFileFomChemForBasis(stepperName, meshFileName, samplingFreq):
  # copy template
  os.system('cp input.template input.txt')

  os.system("sed -i '' s@problemNameValue@chemABC@g input.txt")
  os.system("sed -i '' s@meshFileNameValue@"  + meshFileName            + "@g input.txt")

  os.system("sed -i '' s@odeStepperNameValue@"+ stepperName             + "@g input.txt")
  os.system("sed -i '' s@dtValue@"            + str(cch.dt)             + "@g input.txt")
  os.system("sed -i '' s@finalTimeValue@"     + str(cch.finalTime)      + "@g input.txt")

  os.system("sed -i '' s@diffusionValue@"     + str(cch.diffusion)      + "@g input.txt")
  os.system("sed -i '' s@chemReactionValue@"  + str(cch.chemReaction)   + "@g input.txt")

  os.system("sed -i '' s@observerOnValue@"    + str(1)                  + "@g input.txt")
  os.system("sed -i '' s@shapshotsFreqValue@" + str(samplingFreq)       + "@g input.txt")
  os.system("sed -i '' s@shapshotsFileNameValue@snapshots.txt@g input.txt")
  os.system("sed -i '' s@basisFileNameValue@basis.txt@g input.txt")
  os.system("sed -i '' s@romOnValue@"    + str(0)                       + "@g input.txt")


def createInputFileFomChemForLSPGFullMesh(stepperName, meshFileName, romSize):
  # copy template
  os.system('cp input.template input.txt')

  os.system("sed -i '' s@problemNameValue@chemABC@g input.txt")
  os.system("sed -i '' s@meshFileNameValue@"    + meshFileName          + "@g input.txt")

  os.system("sed -i '' s@odeStepperNameValue@"  + stepperName           + "@g input.txt")
  os.system("sed -i '' s@dtValue@"              + str(cch.dt)           + "@g input.txt")
  os.system("sed -i '' s@finalTimeValue@"       + str(cch.finalTime)    + "@g input.txt")

  os.system("sed -i '' s@diffusionValue@"       + str(cch.diffusion)    + "@g input.txt")
  os.system("sed -i '' s@chemReactionValue@"    + str(cch.chemReaction) + "@g input.txt")

  os.system("sed -i '' s@observerOnValue@"      + str(0)                + "@g input.txt")
  os.system("sed -i '' s@basisFileNameValue@basis.txt@g input.txt")
  os.system("sed -i '' s@romOnValue@"           + str(1)                + "@g input.txt")
  os.system("sed -i '' s@romSizeValue@"         + str(romSize)          + "@g input.txt")


def createInputFileFomChemForLSPGSampleMesh(stepperName, meshFileName, romSize, gidMapFile):
  createInputFileFomChemForLSPGFullMesh(stepperName, meshFileName, romSize)
  os.system("sed -i '' s@smToFmGIDMappingFileNameValue@"    + gidMapFile      + "@g input.txt")
