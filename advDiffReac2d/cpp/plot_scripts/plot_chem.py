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

nx = 50
ny = nx
dt = 0.005

startFrom = int(sys.argv[1])
endAt = int(sys.argv[2])
freq = int(sys.argv[3])

def getXY():
    dd = np.loadtxt("xy.txt")
    x,y = dd[:,0], dd[:,1]
    return [x,y,x.reshape(ny,nx), y.reshape(ny,nx)]

[x,y,xrs,yrs] = getXY()

for i in range(startFrom, endAt, freq):
  print("step = ", i, " time = ", i*dt)
  d0 = np.loadtxt("sol_"+str(i)+".txt")
  c0 = d0[0:-1:3]
  c1 = d0[1:-1:3]
  c2 = d0[2:len(d0)+1:3]

  print(" min/max = ", np.min(c0), np.max(c0))
  print(" min/max = ", np.min(c1), np.max(c1))
  print(" min/max = ", np.min(c2), np.max(c2))
  c0rs = c0.reshape(ny,nx)
  c1rs = c1.reshape(ny,nx)
  c2rs = c2.reshape(ny,nx)

  fig = plt.figure(1)
  ax1 = fig.add_subplot(131)
  ax2 = fig.add_subplot(132)
  ax3 = fig.add_subplot(133)

  nLevs = 25
  lev1 = np.linspace(np.min(c0), np.max(c0), nLevs)
  ax1.imshow(c0rs, cmap=cm.jet, origin='lower',interpolation='bicubic')
  ax1.get_xaxis().set_visible(False)
  ax1.get_yaxis().set_visible(False)
  #plt.contourf(x,y,c0, lev1,cmap=cm.jet)
  #plt.plot(x,y,'ok', markersize=5)
  #ax = plt.gca()
  ax1.set_aspect(aspect=1)

  ax2.imshow(c1rs, cmap=cm.brg, origin='lower',interpolation='bicubic')
  ax2.get_xaxis().set_visible(False)
  ax2.get_yaxis().set_visible(False)
  ax2.set_aspect(aspect=1)

  ax3.imshow(c2rs, cmap=cm.brg, origin='lower',interpolation='bicubic')
  ax3.get_xaxis().set_visible(False)
  ax3.get_yaxis().set_visible(False)
  ax3.set_aspect(aspect=1)

  #plt.clim(300,1800)
  # plt.xlim(0,1.8)
  # plt.ylim(0,0.9)
  plt.pause(0.01)
  #plt.close()

plt.show()
