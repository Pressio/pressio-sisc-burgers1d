#!/usr/bin/env python

import sys, os, time, re
import numpy as np
from subprocess import Popen, list2cmdline, PIPE
import os.path

dt = 0.0005
numSteps = 2048
finalTime = numSteps*dt

# mesh sizes
mesh_sizes = np.array([512, 1024, 2048]) #, 4096, 8192, 16384])
# number of mesh sizes
num_meshes = len(mesh_sizes)

# rom sizes: remember that ROM size has to be smaller than mesh
rom_sizes = np.array([32, 64, 128]) #, 256])
# number of rom sizes
num_rom_sizes = len(rom_sizes)

# number of samples to run to compute timing statistics
numSamplesForTiming = 3

# regex for getting timing from code output
# \d{1,} match one or more (any) digits before the .
# \d{9,} match nice or more (any) digits
timerRegExp = re.compile(r'Elapsed time: \d{1,}.\d{9,}')
