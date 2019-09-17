

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
  print ("max/min c0 "     , dic['c0'][0], dic['c0'][1])
  print ("max/min c1 "     , dic['c1'][0], dic['c1'][1])
  print ("max/min c2 "     , dic['c2'][0], dic['c2'][1])
  print("\n")

def plotImage(state, ax, title, cmName):
  ax.set_title(title, fontsize=12)
  ax.imshow(state, cmap=cmName, origin='lower')#,interpolation='bicubic')
  ax.get_xaxis().set_visible(False)
  ax.get_yaxis().set_visible(False)
  ax.set_aspect(aspect=1)

def plotScatter(x, y, state, ax, title, cmName, vminIn, vmaxIn):
  ax.set_title(title, fontsize=12)
  ax.scatter(x, y, c=state, cmap=cmName, marker='s',
             edgecolors='face', vmin=vminIn, vmax=vmaxIn)
  ax.get_xaxis().set_visible(False)
  ax.get_yaxis().set_visible(False)
  ax.set_xlim([0,1])
  ax.set_ylim([0,1])
  ax.set_aspect(aspect=1)


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
  sol_rom_sm = np.loadtxt(romDataDir+"/xFomReconstructed.txt")
  [c0_rom, c1_rom, c2_rom] = myutc.splitStateBySpeciesNoReshape(sol_rom_sm)
  # compute bounds rom
  print("Bounds for ROM solution reconstructed over sample mesh")
  boundsDicRom = computeBounds(c0_rom, c1_rom, c2_rom)
  printBounds(boundsDicRom)

  # ROM reconstructed over the full mesh
  needToPlotROMoverFull = False
  if os.path.isfile(romDataDir+"/xFomReconstructedFM.txt"):
    needToPlotROMoverFull = True
    # load the reconstructed FOM solution over full mesh
    sol_rom_fm = np.loadtxt(romDataDir+"/xFomReconstructedFM.txt")
    [c0_rom_fm, c1_rom_fm, c2_rom_fm] = myutc.splitStateBySpeciesNoReshape(sol_rom_fm)
    # compute bounds rom
    print("Bounds for ROM solution reconstructed over full mesh")
    boundsDicRom2 = computeBounds(c0_rom_fm, c1_rom_fm, c2_rom_fm)
    printBounds(boundsDicRom2)

  # -------------------------------------------------------------
  # deal with FOM
  # -------------------------------------------------------------
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

  # -------------------------------------------------------------
  # plotting
  # -------------------------------------------------------------
  fig = plt.figure(1)
  if needToPlotROMoverFull:
    ax00,ax01,ax02 = fig.add_subplot(331), fig.add_subplot(332), fig.add_subplot(333)
    ax10,ax11,ax12 = fig.add_subplot(334), fig.add_subplot(335), fig.add_subplot(336)
    ax20,ax21,ax22 = fig.add_subplot(337), fig.add_subplot(338), fig.add_subplot(339)
  else:
    ax00,ax01,ax02 = fig.add_subplot(231), fig.add_subplot(232), fig.add_subplot(233)
    ax10,ax11,ax12 = fig.add_subplot(234), fig.add_subplot(235), fig.add_subplot(236)

  # plot the FOM states top
  plotImage(c0rs_fom, ax00, "$c_0$ FOM", cm.jet)
  plotImage(c1rs_fom, ax01, "$c_1$ FOM", cm.brg)
  plotImage(c2rs_fom, ax02, "$c_2$ FOM", cm.terrain)

  # plot ROM over sample mesh at middle
  plotScatter(x_rom, y_rom, c0_rom, ax10, "$c_0$ ROM",
              cm.jet, bdDicFom['c0'][0], bdDicFom['c0'][1])
  plotScatter(x_rom, y_rom, c1_rom, ax11, "$c_1$ ROM",
              cm.brg, bdDicFom['c1'][0], bdDicFom['c1'][1])
  plotScatter(x_rom, y_rom, c2_rom, ax12, "$c_2$ ROM",
              cm.terrain, bdDicFom['c2'][0], bdDicFom['c2'][1])

  # plot ROM over full mesh at bottom
  if needToPlotROMoverFull:
    plotScatter(x_fom, y_fom, c0_rom_fm, ax20, "$c_0$ ROM - over full",
                cm.jet, bdDicFom['c0'][0], bdDicFom['c0'][1])
    plotScatter(x_fom, y_fom, c1_rom_fm, ax21, "$c_1$ ROM - over full",
                cm.brg, bdDicFom['c1'][0], bdDicFom['c1'][1])
    plotScatter(x_fom, y_fom, c2_rom_fm, ax22, "$c_2$ ROM - over full",
                cm.terrain, bdDicFom['c2'][0], bdDicFom['c2'][1])

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
