

import matplotlib.pyplot as plt
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
from math import sqrt
import os.path
from numpy import linspace, meshgrid
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D
from argparse import ArgumentParser

import constants_chem as cch
import myutils_common as myutc
import plot_common as pltcomm

def loadXY(dest):
  print(dest)
  dd = np.loadtxt(dest+"/xy.txt")
  x,y = dd[:,0], dd[:,1]
  return [x,y]

def computeBounds(c0,c1,c2):
  return {'c0': [np.min(c0), np.max(c0)],
          'c1': [np.min(c1), np.max(c1)],
          'c2': [np.min(c2), np.max(c2)]}

def printBounds(dic):
  print("-------")
  print ("max/min c0 "     , dic['c0'][0], dic['c0'][1])
  print ("max/min c1 "     , dic['c1'][0], dic['c1'][1])
  print ("max/min c2 "     , dic['c2'][0], dic['c2'][1])
  print("\n")


def plotSingleSnapshot(n, romDataDir, fomDataDir, dt):
  # -------------------------------------------------------------
  # deal with ROM first: remember that rom might have sample mesh
  # -------------------------------------------------------------
  print ("")
  print ("Loading ROM results")
  print ("")
  # load xy for ROM
  [x_rom, y_rom] = loadXY(romDataDir)
  # load the reconstructed FOM solution
  sol_rom = np.loadtxt(romDataDir+"/xFomReconstructed.txt")
  # split state by species
  [c0_rom, c1_rom, c2_rom] = myutc.splitStateBySpeciesNoReshape(sol_rom)
  # compute bounds rom
  boundsDicRom = computeBounds(c0_rom, c1_rom, c2_rom)
  printBounds(boundsDicRom)

  # -------------------------------------------------------------
  # deal with FOM
  # -------------------------------------------------------------
  print ("")
  print ("Loading FOM results")
  print ("")
  # load xy for ROM
  [x_fom, y_fom, xrs_fom, yrs_fom] = pltcomm.loadXY(fomDataDir, n)
  # load snapshots
  snaps = np.loadtxt(fomDataDir+"/snapshots.txt")
  # get only target state from snapshot matrix
  targetState = snaps[:, -1]
  # split state by species
  [c0_fom,c1_fom,c2_fom,c0rs_fom,c1rs_fom,c2rs_fom] = myutc.splitStateBySpecies(targetState, n)
  bdDicFom = computeBounds(c0_fom, c1_fom, c2_fom)
  printBounds(bdDicFom)

  # plotting
  fig = plt.figure(1)
  ax00,ax01,ax02 = fig.add_subplot(231), fig.add_subplot(232), fig.add_subplot(233)
  ax10,ax11,ax12 = fig.add_subplot(234), fig.add_subplot(235), fig.add_subplot(236)

  # plot the FOM states
  ax00.set_title("$c_0$ FOM", fontsize=12)
  ax00.imshow(c0rs_fom, cmap=cm.jet, origin='lower')#,interpolation='bicubic')
  ax00.get_xaxis().set_visible(False)
  ax00.get_yaxis().set_visible(False)
  ax00.set_aspect(aspect=1)

  ax01.set_title("$c_1$ FOM", fontsize=12)
  ax01.imshow(c1rs_fom, cmap=cm.brg, origin='lower')#,interpolation='bicubic')
  ax01.get_xaxis().set_visible(False)
  ax01.get_yaxis().set_visible(False)
  ax01.set_aspect(aspect=1)

  ax02.set_title("$c_2$ FOM", fontsize=12)
  ax02.imshow(c2rs_fom, cmap=cm.terrain, origin='lower')#,interpolation='bicubic')
  ax02.get_xaxis().set_visible(False)
  ax02.get_yaxis().set_visible(False)
  ax02.set_aspect(aspect=1)

  # plot ROM at top
  ax10.set_title("$c_0$ from ROM", fontsize=12)
  ax10.scatter(x_rom, y_rom, c=c0_rom, cmap=cm.jet, marker='s', edgecolors='face',
               vmin=bdDicFom['c0'][0], vmax=bdDicFom['c0'][1])
  ax10.get_xaxis().set_visible(False)
  ax10.get_yaxis().set_visible(False)
  ax10.set_xlim([0,1])
  ax10.set_ylim([0,1])
  ax10.set_aspect(aspect=1)

  ax11.set_title("$c_1$ from ROM", fontsize=12)
  ax11.scatter(x_rom, y_rom, c=c1_rom, cmap=cm.brg, marker='s', edgecolors='face',
               vmin=bdDicFom['c1'][0], vmax=bdDicFom['c1'][1])
  ax11.get_xaxis().set_visible(False)
  ax11.get_yaxis().set_visible(False)
  ax11.set_xlim([0,1])
  ax11.set_ylim([0,1])
  ax11.set_aspect(aspect=1)

  ax12.set_title("$c_2$ from ROM", fontsize=12)
  ax12.scatter(x_rom, y_rom, c=c2_rom, cmap=cm.terrain, marker='s', edgecolors='face',
               vmin=bdDicFom['c2'][0], vmax=bdDicFom['c2'][1])
  ax12.get_xaxis().set_visible(False)
  ax12.get_yaxis().set_visible(False)
  ax12.set_xlim([0,1])
  ax12.set_ylim([0,1])
  ax12.set_aspect(aspect=1)

  plt.show()


################################################
# MAIN
################################################
if __name__== "__main__":
  parser = ArgumentParser()
  parser.add_argument("-n", "--n", type=int, dest="n", help="number of cells along one axis")
  parser.add_argument("-rom-data-dir", "--rom-data-dir", type=str, dest="romDataDir", help="rom data directory")
  parser.add_argument("-fom-data-dir", "--fom-data-dir", type=str, dest="fomDataDir", help="fom data directory")
  args = parser.parse_args()

  # make sure the ROM run is compatibel with FOM run
  dt_rom = myutc.extractDtFromTargetInputFile(args.romDataDir)
  finalTime_rom = myutc.extractFinalTimeFromTargetInputFile(args.romDataDir)
  diff_rom = myutc.extractDiffusionFromTargetInputFile(args.romDataDir)
  chemReac_rom = myutc.extractChemReactionFromTargetInputFile(args.romDataDir)

  # make sure the ROM run is compatibel with FOM run
  dt_fom = myutc.extractDtFromTargetInputFile(args.fomDataDir)
  finalTime_fom = myutc.extractFinalTimeFromTargetInputFile(args.fomDataDir)
  diff_fom = myutc.extractDiffusionFromTargetInputFile(args.fomDataDir)
  chemReac_fom = myutc.extractChemReactionFromTargetInputFile(args.fomDataDir)

  assert(dt_rom == dt_fom)
  assert(finalTime_rom == finalTime_fom)
  assert(diff_rom == diff_fom)
  assert(chemReac_rom == chemReac_fom)
  print("ROM and FOM runs have compatible parameters")

  plotSingleSnapshot(args.n, args.romDataDir, args.fomDataDir, dt_fom)
