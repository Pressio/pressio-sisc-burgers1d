#!/usr/bin/env python

import sys, os, time, re
import numpy as np
from subprocess import Popen, list2cmdline, PIPE
import os.path

#------------------------------------------------------
# constants to use for the manuf solution problem
#------------------------------------------------------

dt = 0.002
numSteps = 1000
finalTime = numSteps*dt

diffusion = 0.01
# the chem Reaction must be zero for this test
chemReaction = 0.0

# freq to sample the state
samplingFreq = 10

# mesh sizes
numCell_cases = np.array([8])

# true solution for manuf sol problem
def msTrueSolution(x,y,t):
  s1 = np.sin(2.*np.pi*x)*t*np.cos(4.*np.pi*y)
  s2 = np.sin(4.*np.pi*x)*t*np.sin(4.*np.pi*y)
  s3 = np.sin(8.*np.pi*x)*t*np.cos(2.*np.pi*y)
  return [s1,s2,s3]
