#!/usr/bin/env python

import sys, os, time
import subprocess, math
import numpy as np
import os.path, re
from scipy import stats
from argparse import ArgumentParser
import matplotlib.pyplot as plt

np.set_printoptions(edgeitems=10, linewidth=100000)

def main(fomFile, romFile, meshSize):
  # check that files exists
  if not os.path.isfile(fomFile):
    raise Exception('The file {} does not exist. '.format(fomFile))
  if not os.path.isfile(romFile):
    raise Exception('The file {} does not exist. '.format(romFile))

  # the x coords
  xL, xR = 0., 100.
  dx = (xR-xL)/float(meshSize)
  x = np.linspace(xL, xR, meshSize) + dx*0.5

  # load data
  fomY, romY = np.loadtxt(fomFile), np.loadtxt(romFile)
  fig, ax = plt.subplots()
  plt.plot(x, fomY, '-ob')
  plt.plot(x, romY, '-or', markerfacecolor='none')

  plt.show()

#////////////////////////////////////////////
if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-fom", "--fom", dest="fomFile")
  parser.add_argument("-rom",  "--rom", dest="romFile")
  parser.add_argument("-mesh-size",  "--mesh-size", dest="meshSize")
  args = parser.parse_args()
  main(args.fomFile, args.romFile, args.meshSize)
#////////////////////////////////////////////
