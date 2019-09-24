#!/usr/bin/env python

import sys, os, time, re
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
import os.path

import constants_jphi as cjp

def createInputFileGemm(meshFileName, romSize):
  # copy template
  os.system('cp input.template input.txt')

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("problemNameValue",       "chemABC")
  data = data.replace("meshFileNameValue",      meshFileName)

  # these are just dummy values since for gemm they are not used
  data = data.replace("dtValue",                str(0.1) )
  data = data.replace("finalTimeValue",         str(0.1) )
  data = data.replace("diffusionValue",         str(0.1) )
  data = data.replace("chemReactionValue",      str(0.1) )

  data = data.replace("observerOnValue",        str(0) )
  data = data.replace("basisFileNameValue",     "basis.txt")
  data = data.replace("romOnValue",             str(1) )
  data = data.replace("romSizeValue",           str(romSize) )
  fin.close()
  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()
