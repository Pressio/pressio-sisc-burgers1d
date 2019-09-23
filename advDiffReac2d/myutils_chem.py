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

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("problemNameValue",       "chemABC")
  data = data.replace("meshFileNameValue",      meshFileName)
  data = data.replace("odeStepperNameValue",    stepperName)
  data = data.replace("dtValue",                str(cch.dt) )
  data = data.replace("finalTimeValue",         str(cch.finalTime) )
  data = data.replace("diffusionValue",         str(cch.diffusion) )
  data = data.replace("chemReactionValue",      str(cch.chemReaction) )
  data = data.replace("observerOnValue",        str(0) )
  data = data.replace("romOnValue",             str(0) )
  fin.close()

  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()


def createInputFileFomChemForBasis(stepperName, meshFileName, samplingFreq):
  # copy template
  os.system('cp input.template input.txt')

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("problemNameValue",       "chemABC")
  data = data.replace("meshFileNameValue",      meshFileName)
  data = data.replace("odeStepperNameValue",    stepperName)
  data = data.replace("dtValue",                str(cch.dt) )
  data = data.replace("finalTimeValue",         str(cch.finalTime) )
  data = data.replace("diffusionValue",         str(cch.diffusion) )
  data = data.replace("chemReactionValue",      str(cch.chemReaction) )
  data = data.replace("observerOnValue",        str(1) )
  data = data.replace("shapshotsFreqValue",     str(samplingFreq) )
  data = data.replace("shapshotsFileNameValue", "snapshots.txt" )
  data = data.replace("basisFileNameValue",     "basis.txt")
  data = data.replace("romOnValue",             str(0) )
  fin.close()

  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()



def createInputFileFomChemForLSPGFullMesh(stepperName, meshFileName, romSize):
  # copy template
  os.system('cp input.template input.txt')

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("problemNameValue",       "chemABC")
  data = data.replace("meshFileNameValue",      meshFileName)
  data = data.replace("odeStepperNameValue",    stepperName)
  data = data.replace("dtValue",                str(cch.dt) )
  data = data.replace("finalTimeValue",         str(cch.finalTime) )
  data = data.replace("diffusionValue",         str(cch.diffusion) )
  data = data.replace("chemReactionValue",      str(cch.chemReaction) )
  data = data.replace("observerOnValue",        str(0) )
  data = data.replace("basisFileNameValue",     "basis.txt")
  data = data.replace("romOnValue",             str(0) )
  data = data.replace("romSizeValue",           str(romSize) )
  fin.close()

  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()


def createInputFileFomChemForLSPGSampleMesh(stepperName, meshFileName, romSize, gidMapFile):
  createInputFileFomChemForLSPGFullMesh(stepperName, meshFileName, romSize)

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("smToFmGIDMappingFileNameValue", gidMapFile)
  fin.close()
  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()
