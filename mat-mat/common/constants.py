#!/usr/bin/env python

import sys, os, time, re
import numpy as np
from subprocess import Popen, list2cmdline, PIPE
import os.path

# store current directory
pwd = os.getcwd()

# sizes
matRowSizes = np.array([10000])
matColSizes = np.array([50, 100, 200])

# of times we do mat mat prod inside code
numReplicas = 100


# regex for getting code output
# \d{1,} match one or more (any) digits before the .
# \d{9,} match nine or more (any) digits
timerRegExp   = re.compile(r'Elapsed time: \d{1,}.\d{9,}')
avgTimeRegExp = re.compile(r'Avg time per run: \d{1,}.\d{9,}')
gflopRegExp   = re.compile(r'GFlopPerItem: \d{1,}.\d{5,}')
gflopsRegExp  = re.compile(r'GFlop/sec: \d{1,}.\d{5,}')
