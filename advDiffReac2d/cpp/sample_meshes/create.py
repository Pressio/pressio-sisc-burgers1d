
import matplotlib.pyplot as plt
import sys, os, time
import numpy as np
from numpy import linspace, meshgrid
from matplotlib.mlab import griddata
from matplotlib import cm
import collections

#--------------------------------------
# Cell-centered grid, periodic BC
# natural row ordering
#--------------------------------------

def plotFullGrid(x,y,gids,figId):
  fig = plt.figure(figId)
  ax = plt.gca()
  plt.plot(x,y,'o',markersize=3)
  for i in range(0, len(x)):
    plt.text(x[i], y[i], str(int(gids[i])),verticalalignment='center')
  ax.set_aspect(aspect=1)


def printDicPretty(d):
  for key, value in d.items():
    print(str(key), value)

# lower-left corner is where i,j=(0,0)
def globalIDToGiGj(ID, nx):
    return [ID % nx, int(ID/nx)]

# lower-left corner is where origin is
def fullGrid(nx, ny, numCells, dx, dy):
    x,y,gids = np.zeros(numCells), np.zeros(numCells), np.zeros(numCells)
    ox, oy = 0.5*dx, 0.5*dy
    for i in range(numCells):
        [gi, gj] = globalIDToGiGj(i, nx)
        print (i, gi, gj)
        x[i] = ox + gi * dx
        y[i] = oy + gj * dy
        gids[i] = i
    return [x,y,gids]

def buildFullMeshGraph(nx, ny, numCells):
    G = {}
    # neighbors are listed as east, north, west, south
    # since we have PBC, ensure these are met
    for iPt in range(numCells):
      # convert from globa enumeration to (i,j)
      [gi, gj] = globalIDToGiGj(iPt, nx)
      # temporary list holding neighbors
      tmpList = np.zeros(4)

      # west neighbor
      # if gi==0, we are on left BD
      if gi==0: tmpList[0]=iPt+nx-1
      if gi>0 : tmpList[0]=iPt-1

      # north naighbor
      # if gj==ny-1, we are on TOP BD
      if gj==ny-1: tmpList[1]=iPt-nx*(ny-1)
      if gj<ny-1 : tmpList[1]=iPt+nx

      # east neighbor
      # if gi==nx-1, we are on Right BD
      if gi==nx-1: tmpList[2]=iPt-nx+1
      if gi<nx-1 : tmpList[2]=iPt+1

      # south naighbor
      # if gj==0, we are on bottom BD
      if gj==0: tmpList[3]=iPt+nx*(ny-1)
      if gj>0 : tmpList[3]=iPt-nx

      # store currrent neighboring list
      G[iPt] = tmpList
    return G


def createListTargetResidualGIDs(nx, ny, numCells, x, y):
    L = []
    for i in range(numCells):
      L.append(i)
      #if (x[i] < 1.1 or x[i] > 1.6): L.append(i)
    return L


L = [1., 1.]
Nx, Ny = 5,5
numCells = Nx*Ny
dx,dy = L[0]/Nx, L[1]/Ny

# enumerate figures
figId = 0

# full grid
[x,y,gids] = fullGrid(Nx, Ny, numCells, dx, dy)
plotFullGrid(x,y,gids,figId)

# for each grid point, build list of neighbors
# this is easy because it is structured mesh with natural row ordering
# i.e. left-right, bottom-top
fullMeshGraph = buildFullMeshGraph(Nx, Ny, numCells)
printDicPretty(fullMeshGraph)

# create list of GIDs where we want to compute residual
# the GIDs must be wrt to the full mesh
rGIDs = createListTargetResidualGIDs(Nx, Ny, numCells, x, y)
#rGIDs = [3, 14, 19, 20, 28, 38, 54]
#rGIDs = [3, 10, 20, 24, 29, 30, 40, 49, 50, 54, 60, 69, 70, 80, 93]
#rGIDs = [6, 15, 20, 35]
#np.savetxt('r_gids.dat', rGIDs, fmt='%10i')

# store the connectivity for the target residual points
# using global indices wrt the full mesh
residGraph = collections.OrderedDict()
for rPt in rGIDs:
    residGraph[rPt] = fullMeshGraph[rPt]
print("residGraph")
for k,v in residGraph.items(): print(k, v)
print("\n")

# residGraph baiscally containes the points needed for the sample mesh
# it contains both those where we need to compute the residual as well as
# the points where the state is needed
# The sample mesh graph is defined in terms of the GIDs of the full mesh.
# So now we need to create a list of UNIQUE GIDs, the GIDs of all
# cells where we need state and residuals
# Basically we need to enumerate all the sample mesh points

# first, we extract from the graph all unique GIDs since
# there might be duplicates because of the connectivity
allGIDs = []
# loop over target cells wherw we want residual
for k,v in residGraph.items():
    # append the GID of this cell
    allGIDs.append(k)
    # append GID of stencils/neighborin cells
    for j in v:
      allGIDs.append(int(j))

# remove duplicates and sort
allGIDs = list(dict.fromkeys(allGIDs))
allGIDs.sort()
print("allGIDs")
print(allGIDs)
print("\n")

# enumerate the sample mesh points with new indexing
# and create a map of full-mesh gids to new gids
# maybe we need to explore different ordering of sample mesh
# fm_to_sm_map is such that:
# - key   = GID wrt full mesh
# - value = GID wrt sample mesh
fm_to_sm_map = collections.OrderedDict()
i = 0
for pt in allGIDs:
  fm_to_sm_map[pt] = i
  i+=1
print("Done with fm_to_sm_map")
for k,v in fm_to_sm_map.items(): print(k, v)

print("doing now the sm -> fm gids mapping ")
sm_to_fm_map = collections.OrderedDict()
for k,v in fm_to_sm_map.items():
  sm_to_fm_map[v] = k
for k,v in sm_to_fm_map.items(): print(k, v)
print("Done with sm_to_fm_map")


# now that we have a list of unique GIDs for the sample mesh,
# convert the gids of the original connectivity graph from full to sample mesh indexing
residGraphSM = collections.OrderedDict()
for rGidFM, v in residGraph.items():
    smGID = fm_to_sm_map[rGidFM]
    smStencilGIDs = v
    for i in range(len(smStencilGIDs)):
        thisGID = smStencilGIDs[i]
        smStencilGIDs[i] = fm_to_sm_map[thisGID]
    residGraphSM[smGID] = smStencilGIDs

print("\n")
print("Done with residGraphSM")
for k,v in residGraphSM.items(): print(k, v)

# number of pts where we compute residual
numResidualPts = len(residGraphSM)
print ("numResidualPts = ", numResidualPts)
# total number of state cells, which is basically the sample mesh size
numStatePts = len(fm_to_sm_map)
print ("numStatePts    = ", numStatePts)

# print mesh file
f = open("mesh.dat","w+")
f.write("dx %f\n" % dx)
f.write("dy %f\n" % dy)
f.write("numResidualPts %d\n" % numResidualPts)
f.write("numStatePts %d\n" % numStatePts)
for k in sorted(residGraphSM.keys()):
  f.write("%d " % k)
  f.write("%f " % x[sm_to_fm_map[k]])
  f.write("%f " % y[sm_to_fm_map[k]])
  for i in residGraphSM[k]:
    # print gid
    f.write("%d " % i)
    # pring coords
    f.write("%f " % x[sm_to_fm_map[i]])
    f.write("%f " % y[sm_to_fm_map[i]])
  f.write("\n")
f.close()

# # print gids mapping
# f1 = open("sm_to_full_gid_map.dat","w+")
# for k,v in fm_to_sm_map.items():
#     f1.write("%d %d\n" % (v, k))
# f1.close()

# keep only subset of x,y that belongs to sample mesh
x_sm, y_sm = x[ list(fm_to_sm_map.keys()) ], y[ list(fm_to_sm_map.keys()) ]

figId+=1
fig = plt.figure(figId)
ax = plt.gca()
plt.scatter(x,y,marker='s', s=50)
plt.scatter(x_sm,y_sm, marker='o', c='r', s=60, facecolor='None')
ax.set_aspect(aspect=1)
plt.show()
