#!/usr/bin/env python

import sys, os, time, re
import numpy as np
from subprocess import Popen, list2cmdline, PIPE
import os.path

#------------------------------------------------------
# constants to use for the chemical problem
#------------------------------------------------------

dt = 0.005
numSteps = 1000
finalTime = numSteps*dt

diffusion = 0.001
chemReaction = 10.

# mesh sizes
numCell_cases = np.array([5])

# rom sizes: remember that ROM size has to be smaller than mesh
romSize_cases = np.array([10])#, 50, 100])

# number of samples to run to compute timing statistics
numSamplesForTiming = 1

# regex for getting timing from code output
# \d{1,} match one or more (any) digits before the .
# \d{9,} match nice or more (any) digits
timerRegExp = re.compile(r'Elapsed time: \d{1,}.\d{9,}')
