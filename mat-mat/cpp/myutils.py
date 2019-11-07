#!/usr/bin/env python

import sys, os, time, re
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
import os.path
import constants

# store current directory
pwd = os.getcwd()

def createInputFile(numDofs, romSize, numReplicas):
  # copy template
  os.system('cp input.template input.txt')

  fin = open("input.txt", "rt")
  data = fin.read()
  data = data.replace("numDofsValue", str(numDofs) )
  data = data.replace("romSizeValue", str(romSize) )
  data = data.replace("numReplicasValue", str(numReplicas) )
  fin.close()
  fin = open("input.txt", "wt")
  fin.write(data)
  fin.close()
