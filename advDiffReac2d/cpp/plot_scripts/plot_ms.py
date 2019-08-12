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
dt = 0.01

startFrom = int(sys.argv[1])
endAt = int(sys.argv[2])
freq = int(sys.argv[3])


def computeTrue(x,y,t):
  s1 = np.sin(2.*np.pi*x)*t*np.cos(4.*np.pi*y)
  s2 = np.sin(4.*np.pi*x)*t*np.sin(4.*np.pi*y)
  s3 = np.sin(8.*np.pi*x)*t*np.cos(2.*np.pi*y)
  return [s1,s2,s3]

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

  [u0,u1,u2] = computeTrue(x,y, i*dt)
  u0rs = u0.reshape(ny,nx)
  u1rs = u1.reshape(ny,nx)
  u2rs = u2.reshape(ny,nx)

  print( "c sizes = ", len(c0), len(c1), len(c2))
  print( "trueU size = ", len(u0), len(u1), len(u2))

  print("-------")
  print ("max/min true u0 ", np.min(u0), np.max(u0) )
  print ("max/min c0 ", np.min(c0), np.max(c0))
  print ("error = ", np.sqrt(((u0 - c0) ** 2).mean()))
  print("\n")
  print ("max/min true u1 ", np.min(u1), np.max(u1) )
  print ("max/min c1 ", np.min(c1), np.max(c1))
  print ("error = ", np.sqrt(((u1 - c1) ** 2).mean()))
  print("\n")
  print ("max/min true u2 ", np.min(u2), np.max(u2) )
  print ("max/min c2 ", np.min(c2), np.max(c2))
  print ("error = ", np.sqrt(((u2 - c2) ** 2).mean()))
  print("\n")


  c0rs = c0.reshape(ny,nx)
  c1rs = c1.reshape(ny,nx)
  c2rs = c2.reshape(ny,nx)

  nLevs = 25
  #lev1 = np.linspace(np.min(c0), np.max(c0), nLevs)

  fig = plt.figure(0)
  ax = fig.add_subplot(111, projection='3d')
  ax.plot_wireframe(xrs, yrs, u2rs, rstride=1, cstride=1, color='r')
  ax.plot_wireframe(xrs, yrs, c2rs, rstride=2, cstride=2, color='k')
  # plt.contour(xv, yv, trueU, color='k')
  # #plt.contour(xv, yv, c0, color='r')


  fig = plt.figure(1)
  ax1 = fig.add_subplot(131)
  ax2 = fig.add_subplot(132)
  ax3 = fig.add_subplot(133)

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

  ax3.imshow(c2rs, cmap=cm.brg, origin='lower',interpolation='bicubic')
  ax3.get_xaxis().set_visible(False)
  ax3.get_yaxis().set_visible(False)

  #plt.clim(300,1800)
  # plt.xlim(0,1.8)
  # plt.ylim(0,0.9)
  #plt.colorbar()
  plt.pause(0.01)
  #plt.close()

plt.show()
