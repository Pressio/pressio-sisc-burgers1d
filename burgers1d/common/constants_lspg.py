#!/usr/bin/env python

import sys, os, time, re
import numpy as np
from subprocess import Popen, list2cmdline, PIPE
import os.path

dt = 0.0005
# num of steps to generate basis
numStepsBasis = 1024
# final time
finalTimeBasis = numStepsBasis*dt

# mesh sizes
mesh_sizes = np.array([1024, 2048, 4096, 8192])
# number of mesh sizes
num_meshes = len(mesh_sizes)

# num of steps for the timing (can be mapped to each mesh size
# so that for smaller meshes we run for longer to avoid noise issues on timing)
numStepsTiming = { mesh_sizes[0]: numStepsBasis,
                   mesh_sizes[1]: numStepsBasis/2,
                   mesh_sizes[2]: numStepsBasis/4,
                   mesh_sizes[3]: numStepsBasis/8}
# final time
finalTimeTiming = {}
for i in range(num_meshes):
  ms = mesh_sizes[i]
  finalTimeTiming[ms] = numStepsTiming[ms]*dt
print(finalTimeTiming)


# rom sizes: remember that ROM size has to be smaller than mesh
rom_sizes = np.array([16,32,64,128,256])
# number of rom sizes
num_rom_sizes = len(rom_sizes)

# number of samples to run to compute timing statistics
numSamplesForTiming = 5

# regex for getting timing from code output
# \d{1,} match one or more (any) digits before the .
# \d{9,} match nice or more (any) digits
timerRegExp = re.compile(r'Elapsed time: \d{1,}.\d{9,}')
