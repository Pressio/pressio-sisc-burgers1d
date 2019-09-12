#!/usr/bin/env python

import sys, os, time
import numpy as np
from math import sqrt

def loadXY(dest, n):
  print(dest)
  dd = np.loadtxt(dest+"/xy.txt")
  x,y = dd[:,0], dd[:,1]
  return [x,y,x.reshape(n,n), y.reshape(n,n)]
