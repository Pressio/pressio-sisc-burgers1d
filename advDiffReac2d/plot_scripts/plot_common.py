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


def splitStateBySpecies(targetState, n):
  c0 = targetState[0:-1:3]
  c1 = targetState[1:-1:3]
  c2 = targetState[2:len(targetState)+1:3]
  c0rs = c0.reshape(n,n)
  c1rs = c1.reshape(n,n)
  c2rs = c2.reshape(n,n)
  return [c0,c1,c2,c0rs,c1rs,c2rs]


def computeTimeOfSnapshot(snapId, samplingFreq, dt):
  # compute time of this snapId: since snapshots do not include the init cond,
  # snapId=0 corresponds to snapshot taken at step = samplingFreq, so to compute
  # the right time, we need to increment snapId by 1 before multiplying
  snapshotTime = (snapId+1) * samplingFreq * dt
  print( "sampling freq is = ", samplingFreq)
  print( "time = ", snapshotTime)
  return snapshotTime


def extractSamplingFreqFromTargetInputFile(dataDir):
  # grab sampling frequency from the input file
  popen = Popen(["grep", "shapshotsFreq", str(dataDir)+"/input.txt"], stdout=PIPE)
  popen.wait()
  output = popen.stdout.read()
  samplingFrequency = int(output.split()[1])
  print ("Inside " + dataDir + ", detected samplingFrequncy = ", samplingFrequency)
  return samplingFrequency


def extractDtFromTargetInputFile(dataDir):
  # grab dt from the input file
  popen = Popen(["grep", "dt", str(dataDir)+"/input.txt"], stdout=PIPE)
  popen.wait()
  output = popen.stdout.read()
  dt = float(output.split()[1])
  print ("Inside " + dataDir + ", detected dt = ", dt)
  return dt
