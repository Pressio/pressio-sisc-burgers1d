
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

from mesh_utils import *

#-------------------------------------------------------
# Cell-centered, natural order mesh with periodic BC
#-------------------------------------------------------

# natural row ordering
#
#  ...
#  10 11 12 13 14
#  5  6  7  8  9
#  0  1  2  3  4
#

class NatOrdMesh:
  def __init__(self, Nx, Ny, dx, dy):
    self.Nx_ = Nx
    self.Ny_ = Ny
    self.numCells_ = Nx * Ny
    self.dx_ = dx
    self.dy_ = dy
    self.x_ = np.zeros(self.numCells_)
    self.y_ = np.zeros(self.numCells_)
    self.gids_ = np.zeros(self.numCells_)
    self.G_ = {}

    self.createFullGrid()
    self.buildGraph()
    #convert the graph dictionary to sparse matrix
    self.spMat_ = convertGraphDicToSparseMatrix(self.G_)

  def getXY(self):
    return [self.x_, self.y_]

  def getGIDs(self):
    return self.gids_

  # return the grap (dictionary) repr of the connectivity
  def getGraph(self):
    return self.G_

  # return the sparse matrix repr of the connectivity
  def getSparseMatrixRepr(self):
    return self.spMat_

  # lower-left corner is where i,j=(0,0)
  def globalIDToGiGj(self,ID):
    return [ID % self.Nx_, int(ID/self.Nx_)]

  # lower-left corner is where origin is
  def createFullGrid(self):
    ox, oy = 0.5*self.dx_, 0.5*self.dy_
    for i in range(self.numCells_):
      [gi, gj] = self.globalIDToGiGj(i)
      self.x_[i] = ox + gi * self.dx_
      self.y_[i] = oy + gj * self.dy_
      self.gids_[i] = i

  def buildGraph(self):
    # neighbors are listed as east, north, west, south
    # since we have PBC, ensure these are met
    for iPt in range(self.numCells_):
      # convert from globa enumeration to (i,j)
      [gi, gj] = self.globalIDToGiGj(iPt)

      # temporary list holding neighbors
      tmpList = np.zeros(4, dtype=int)

      # west neighbor
      # if gi==0, we are on left BD
      if gi==0: tmpList[0]=iPt+self.Nx_-1
      if gi>0 : tmpList[0]=iPt-1

      # north naighbor
      # if gj==self.Ny_-1, we are on TOP BD
      if gj==self.Ny_-1: tmpList[1]=iPt-self.Nx_*(self.Ny_-1)
      if gj<self.Ny_-1 : tmpList[1]=iPt+self.Nx_

      # east neighbor
      # if gi==self.Nx_-1, we are on Right BD
      if gi==self.Nx_-1: tmpList[2]=iPt-self.Nx_+1
      if gi<self.Nx_-1 : tmpList[2]=iPt+1

      # south naighbor
      # if gj==0, we are on bottom BD
      if gj==0: tmpList[3]=iPt+self.Nx_*(self.Ny_-1)
      if gj>0 : tmpList[3]=iPt-self.Nx_

      # store currrent neighboring list
      self.G_[iPt] = tmpList
