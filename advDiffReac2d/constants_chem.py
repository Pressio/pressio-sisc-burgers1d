#!/usr/bin/env python

import sys, os, time, re
import numpy as np
from subprocess import Popen, list2cmdline, PIPE
import os.path

#------------------------------------------------------
# constants to use for the chemical problem
#------------------------------------------------------

dt = 0.002
numSteps = 50
finalTime = numSteps*dt

diffusion = 0.001
chemReaction = 10.

# number of samples to run to compute timing statistics
numSamplesForTiming = 1

# FULL mesh sizes: the number of cells along each axis.
numCell_cases = np.array([32])

# residual sample mesh sizes: size of sample mesh where
# we compute the residual
sampleMesh_sizes = np.array([200])

# rom sizes: in general, you want the rom size to be
# smaller than the meshSize*dofPerCells so that the
# matrix of the basis vectors is a tall-skinny one
romSize_cases = np.array([10])
