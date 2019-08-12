
#!/usr/bin/env python

import matplotlib.pyplot as plt
import sys, os, time
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
import os.path
from numpy import linspace, meshgrid
from matplotlib.mlab import griddata
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D

nx = 50
ny = nx

def getXY():
  dd = np.loadtxt("xy.txt")
  x,y = dd[:,0], dd[:,1]
  x = x.reshape(ny,nx)
  y = y.reshape(ny,nx)
  return x,y

x,y = getXY()

uv = np.loadtxt("uv.txt")
u = uv[:,0].reshape(ny,nx)
v = uv[:,1].reshape(ny,nx)

fig = plt.figure(1)
ax1 = fig.add_subplot(121)
ax2 = fig.add_subplot(122)
#ax3 = fig.add_subplot(133)

#ax1.imshow(psi, cmap=cm.jet, origin='lower')#,interpolation='bicubic')
#ax1.get_xaxis().set_visible(False)
#ax1.get_yaxis().set_visible(False)
#ax1.set_aspect(aspect=1)

#ax2.imshow(u, cmap=cm.jet, origin='lower')#,interpolation='bicubic')
#ax2.get_xaxis().set_visible(False)
#ax2.get_yaxis().set_visible(False)

#ax3.imshow(v, cmap=cm.jet, origin='lower')#,interpolation='bicubic')
ax2.quiver(x,y,u,v,scale=10)
ax2.get_xaxis().set_visible(False)
ax2.get_yaxis().set_visible(False)
ax2.set_aspect(aspect=1)

#ax1 = fig.gca(projection='3d')
#ax.plot_surface(x, y, psi, cmap=cm.jet,
#                linewidth=0, antialiased=False)
# xe,ye,re=0.5,0.5,0.1
# v2 = np.zeros((ny,nx))
# for i in range(0,nx):
#   for j in range(0,ny):
#     num = (x[i][j] - xe)**2 + (y[i][j] - ye)**2
#     den = 2.*re**2
#     v2[j][i] = -(re**2) * np.exp(-num/den)*(x[i][j]-xe)/(re**2)

#fig = plt.figure(2)
#ax1 = fig.gca(projection='3d')
#ax1.plot_surface(x, y, u, cmap=cm.jet,
#                linewidth=0, antialiased=False)
#ax1.plot_wireframe(x, y, v, color='r')
#ax1.plot_wireframe(x, y, v2, color='k')
plt.show()
