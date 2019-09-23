
import matplotlib.pyplot as plt
import sys, os, time
import numpy as np
from numpy import linspace, meshgrid
from matplotlib import cm
import collections
from argparse import ArgumentParser
import random
import scipy.sparse as sp
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import reverse_cuthill_mckee

from natural_order_mesh import NatOrdMesh
from mesh_utils import *

#-------------------------------------------------------
# Cell-centered grid, periodic BC
#-------------------------------------------------------

def main(Nx, Ny, samplingType, targetPct, plotting, orderingType):
  L = [1., 1.]
  numCells = Nx*Ny
  dx,dy = L[0]/Nx, L[1]/Ny

  if (plotting != "none"):
    # figFM is to store the full mesh results
    figFM = plt.figure(0)
    ax00,ax01 = figFM.add_subplot(221), figFM.add_subplot(222)
    ax10,ax11 = figFM.add_subplot(223), figFM.add_subplot(224)
    if samplingType=="random":
      figSM = plt.figure(1)
      bx00,bx01 = figSM.add_subplot(121), figSM.add_subplot(122)

  # ---------------------------------------------
  # generate natural order FULL mesh
  # ---------------------------------------------
  meshObj = NatOrdMesh(Nx, Ny, dx, dy)
  [x,y] = [x, y] = meshObj.getXY()
  gids = meshObj.getGIDs()
  G = meshObj.getGraph()
  spMat = meshObj.getSparseMatrixRepr()

  if plotting != "none":
    plotLabels(x, y, gids, ax00)
  print("natural order full mesh connectivity")
  #printDicPretty(G)
  #print("\n")
  if plotting != "none":
    ax01.spy(spMat)

  # -----------------------------------------------------
  # reorder if needed, using reverse cuthill mckee (RCM)
  # -----------------------------------------------------
  if (orderingType == "rcm"):
    # the starting matrix will always be symmetric because it is full mesh
    # with a symmetric stencil
    [rcmPermInd, spMatRCM] = reverseCuthillMckee(spMat, symmetric=True)

    # hash table to map permutation indices to old indices
    # key = the old cell index
    # value = the new index of the cell after RCM
    ht = {}
    for i in range(len(rcmPermInd)):
      ht[rcmPermInd[i]] = i

    # use the permuation indices to find the new indexing of the cells
    newFullMeshGraph = {}
    for key, value in G.items():
      newFullMeshGraph[ht[key]] = [ht[i] for i in value]
    # sort by key
    newFullMeshGraph = collections.OrderedDict(sorted(newFullMeshGraph.items()))
    print("full mesh connectivity after RCM")
    #printDicPretty(newFullMeshGraph)
    print("\n")

    # this is ugly but works, replace globals
    G = newFullMeshGraph
    spMat = spMatRCM
    #with the permuation indices, update the gids labeling
    x, y = x[rcmPermInd], y[rcmPermInd]
    if plotting != "none":
     plotLabels(x,y,gids,ax10)
     ax11.spy(spMatRCM)


  # -----------------------------------------------------
  # create a list of GIDs where we want to compute residual
  # this can either be all the GIDs if the target mesh is full,
  # or it can be a subset of the cells, if we want a sample mesh.
  # the range of the GIDs is (0, numCells) and must be a unique list
  # because we do not want to sample the same element twice
  # -----------------------------------------------------
  if (samplingType=="full"):
    rGIDs = [i for i in range(numCells)]
  elif (samplingType=="random"):
    if (targetPct < 0 ):
      print("For random sample mesh you need to set --pct=n")
      print("where n>0 and defines the % of full mesh to take")
    # create the random list of cells
    rGIDs = createRandomListTargetResidualGIDs(Nx, Ny, numCells, targetPct)
  else:
    print("unknown value for sample-type")
    sys.exit(1)

  # -----------------------------------------------------
  # store the connectivity for the selected points
  # by extracting it from the mesh constrcuted above
  # -----------------------------------------------------
  subGraph = collections.OrderedDict()
  for rPt in rGIDs:
      subGraph[rPt] = G[rPt]
  print("subGraph")
  #for k,v in subGraph.items(): print(k, v)
  print("\n")

  # the subGraph contains the sample mesh cells.
  # BUT it contains those where we need to compute the residual as well as
  # the cells where only the state is needed.
  # The sample mesh graph is defined in terms of the GIDs of the full mesh.
  # now we need to create a list of UNIQUE GIDs of the subGraph,
  # the GIDs of all cells where we need state and residuals.
  # Basically we need to uniquely enumerate all the sample mesh cells.

  # first, we extract from the graph all unique GIDs since
  # there might be duplicates because of the connectivity
  allGIDs = []
  # loop over target cells wherw we want residual
  for k,v in subGraph.items():
      # append the GID of this cell
      allGIDs.append(k)
      # append GID of stencils/neighborin cells
      for j in v:
        allGIDs.append(int(j))

  # remove duplicates and sort
  allGIDs = list(dict.fromkeys(allGIDs))
  allGIDs.sort()
  print("sample mesh allGIDs")
  #print(allGIDs)
  print("\n")

  # -----------------------------------------------------
  # if we get here, the sample mesh graph contains the GIDs
  # of the sample mesh wrt the FULL mesh ordering becaus we
  # simply extracted a subset from the full mesh.
  # However, what we need is not a new sequential enumeration of the
  # sample mesh cells.
  # We numerate the sample mesh points with new indexing
  # and create a map of full-mesh gids to new gids
  # -----------------------------------------------------
  # fm_to_sm_map is such that:
  # - key   = GID wrt full mesh
  # - value = GID wrt sample mesh
  fm_to_sm_map = collections.OrderedDict()
  i = 0
  for pt in allGIDs:
    fm_to_sm_map[pt] = i
    i+=1
  print("Done with fm_to_sm_map")
  #for k,v in fm_to_sm_map.items(): print(k, v)

  print("doing now the sm -> fm gids mapping ")
  sm_to_fm_map = collections.OrderedDict()
  for k,v in fm_to_sm_map.items():
    sm_to_fm_map[v] = k
  #for k,v in sm_to_fm_map.items(): print(k, v)
  print("Done with sm_to_fm_map")

  # -----------------------------------------------------
  # we have a list of unique GIDs for the sample mesh.
  # Map the GIDs from the full to sample mesh indexing
  # -----------------------------------------------------
  residGraphSM = collections.OrderedDict()
  for rGidFM, v in subGraph.items():
      smGID = fm_to_sm_map[rGidFM]
      smStencilGIDs = v
      for i in range(len(smStencilGIDs)):
          thisGID = smStencilGIDs[i]
          smStencilGIDs[i] = fm_to_sm_map[thisGID]
      residGraphSM[smGID] = smStencilGIDs

  print("\n")
  print("Done with residGraphSM")
  print("sample mesh connectivity")
  #printDicPretty(residGraphSM)
  print("\n")

  gids_sm = list(sm_to_fm_map.keys())

  if plotting != "none" and samplingType == "random":
    # keep only subset of x,y that belongs to sample mesh
    x_sm, y_sm = x[ list(fm_to_sm_map.keys()) ], y[ list(fm_to_sm_map.keys()) ]
    bx00.scatter(x,y,marker='s', s=50)
    plotLabels(x_sm, y_sm, gids_sm, bx00, 's', 'r', 5)
    bx00.set_aspect(aspect=1)

    # convert sample mesh graph to sparse matrix
    sampleMeshSpMat = convertGraphDicToSparseMatrix(residGraphSM)
    bx01.spy(sampleMeshSpMat)

  # # -----------------------------------------------------
  # # the reverse cuthill mckee (RCM) can be applied to sample mesh too?
  # # not sure but it does not work if I pass false for symmetry
  # leave it out for now
  # # -----------------------------------------------------
  # if (orderingType == "rcm"):
  #   # the sample mesh graph is NOT symmetric, so pass false
  #   #[rcmPermInd, spMatRCM] = reverseCuthillMckee(sampleMeshSpMat, False)
  #   # not sure but it does not work even if I pass false for symmetry

  #   # # hash table to map permutation indices to old indices
  #   # # key = the old cell index
  #   # # value = the new index of the cell after RCM
  #   #ht = {}
  #   # for i in range(len(rcmPermInd)):
  #   #   ht[rcmPermInd[i]] = i

  #   # # use the permuation indices to find the new indexing of the cells
  #   # newFullMeshGraph = {}
  #   # for key, value in residGraphSM.items():
  #   #   newFullMeshGraph[ht[key]] = [ht[i] for i in value]
  #   # # sort by key
  #   # newFullMeshGraph = collections.OrderedDict(sorted(newFullMeshGraph.items()))
  #   # print("sample mesh connectivity after RCM")
  #   # printDicPretty(newFullMeshGraph)
  #   # print("\n")

  #   # # replace globals
  #   # residGraphSM = newFullMeshGraph
  #   # sampleMeshSpMat = spMatRCM
  #   # #with the permuation indices, update the gids labeling
  #   # x, y = x_sm[rcmPermInd], y_sm[rcmPermInd]
  #   # if showPlots:
  #   #  plotLabels(x,y,gids_sm,bx10)
  #   #  bx11.spy(spMatRCM)

  # -----------------------------------------------------
  # number of pts where we compute residual
  numResidualPts = len(residGraphSM)
  print ("numResidualPts = ", numResidualPts,
         " which is = ", numResidualPts/numCells*100, " % of full mesh")
  # total number of state cells, which is basically the sample mesh size
  numStatePts = len(fm_to_sm_map)
  print ("numStatePts = ", numStatePts,
         " which is = ", numStatePts/numCells*100, " % of full mesh")

  # -----------------------------------------------------
  # print mesh file
  f = open("mesh.dat","w+")
  f.write("dx %.14f\n" % dx)
  f.write("dy %.14f\n" % dy)
  f.write("numResidualPts %8d\n" % numResidualPts)
  f.write("numStatePts %8d\n" % numStatePts)
  for k in sorted(residGraphSM.keys()):
    f.write("%8d " % k)
    f.write("%.14f " % x[sm_to_fm_map[k]])
    f.write("%.14f " % y[sm_to_fm_map[k]])
    for i in residGraphSM[k]:
      # print gid
      f.write("%8d " % i)
      # pring coords
      f.write("%.14f " % x[sm_to_fm_map[i]])
      f.write("%.14f " % y[sm_to_fm_map[i]])
    f.write("\n")
  f.close()

  # -----------------------------------------------------
  # print gids mapping: mapping between GIDs in sample mesh to full mesh
  # I only need this when I use the sample mesh
  # -----------------------------------------------------
  if (samplingType=="random"):
    f1 = open("sm_to_fm_gid_mapping.dat","w+")
    for k,v in fm_to_sm_map.items():
      f1.write("%8d %8d\n" % (v, k))
    f1.close()

  if plotting == "show":
    plt.show()
  elif plotting =="print":
    figFM.savefig('fullFM.png')
    if samplingType == "random":
      figSM.savefig('fullSM.png')


###############################
if __name__== "__main__":
###############################

  # fix seed so that we have reproducibility
  random.seed( 1474543 )

  parser = ArgumentParser()
  parser.add_argument("-nx", "--nx", type=int, dest="nx")
  parser.add_argument("-ny", "--ny", type=int, dest="ny", default="-1")

  parser.add_argument("-sampling-type", "--sampling-type",
                      dest="samplingType",
                      default="full",
                      help="What type of mesh you need:\n"+
                           "use <full> for creating connectivity for full mesh,\n"+
                           "use <random> for creating connectivity for sample mesh")

  parser.add_argument("-ordering", "--ordering",
                      dest="orderingType",
                      default="natural",
                      help="What type of ordering you want:\n"+
                           "use <natural> for natural ordering of the mesh cells,\n"+
                           "use <rcm> for reverse cuthill-mckee")

  parser.add_argument("-plotting", "--plotting",
                      dest="plotting",
                      default="none",
                      help="What type of plotting you want:\n"+
                           "use <show> for showing plots,\n"+
                           "use <print> for printing only, \n"+
                           "use <none> for no plots")

  parser.add_argument("-pct", "--pct",
                      dest="targetPct",
                      default=-1,
                      help="The target % of elements you want.\n"+
                      "You do not need this if you select -sampling-type=full,\n"+
                      "since in that case we sample the full mesh.\n"+
                      "But you need to set it for -sampling-type=random")
  args = parser.parse_args()

  assert( args.samplingType == "full" or args.samplingType == "random" )
  assert( args.orderingType == "natural" or args.orderingType == "rcm" )

  print("Ordering type = " + args.orderingType)

  if (args.ny==-1): args.ny = args.nx

  # for now, we need to have square grid
  if (args.nx != args.ny):
    print(" currently only supports nx = ny" )
    assert(args.nx == args.ny)

  main(args.nx, args.ny, args.samplingType,
       float(args.targetPct), args.plotting, args.orderingType)
