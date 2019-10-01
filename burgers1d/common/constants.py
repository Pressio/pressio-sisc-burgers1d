#!/usr/bin/env python

import sys, os, time, re
import numpy as np
from subprocess import Popen, list2cmdline, PIPE
import os.path

# store current directory
pwd = os.getcwd()

dt = 0.01
numSteps = 500
finalTime = numSteps*dt

# mesh sizes
numCell_cases = np.array([20000])#, 5000, 10000, 50000, 100000])

# rom sizes: remember that ROM size has to be smaller than mesh
romSize_cases = np.array([50])#, 50, 100])

# number of samples to run to compute timing statistics
numSamplesForTiming = 1

# regex for getting timing from code output
# \d{1,} match one or more (any) digits before the .
# \d{9,} match nice or more (any) digits
timerRegExp = re.compile(r'Elapsed time: \d{1,}.\d{9,}')
