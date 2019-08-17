#!/usr/bin/env python

import matplotlib.pyplot as plt
import sys, os, time
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
from sklearn.metrics import mean_squared_error
from math import sqrt
import os.path
from numpy import linspace, meshgrid
from matplotlib.mlab import griddata
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D
from argparse import ArgumentParser

def loadXY(dest, n):
  print(dest)
  dd = np.loadtxt(dest+"/xy.txt")
  x,y = dd[:,0], dd[:,1]
  return [x,y,x.reshape(n,n), y.reshape(n,n)]
