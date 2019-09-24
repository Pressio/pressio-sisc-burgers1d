#!/usr/bin/env python

import sys, os, time, re
import numpy as np
from subprocess import Popen, list2cmdline, PIPE
import os.path

# number of samples to run to compute timing statistics
numSamplesForTiming = 1

# FULL mesh sizes: the number of cells along each axis.
numCell_cases = np.array([256])

# rom sizes: in general, you want the rom size to be
# smaller than the meshSize*dofPerCells so that the
# matrix of the basis vectors is a tall-skinny one
romSize_cases = np.array([200])
