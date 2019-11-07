
import numpy as np
import sys, time
import timeit
import math
from numba import jit, vectorize, njit, jitclass
from numba import f8, float64, int32
import profile

from burgers1d import Burgers1d
import pressio4pyGalerkin
import pressio4pyOps

def doGalerkinForTargetSteps(nsteps, appObj, yRef, decoder, yRom, t0, ops):
  galerkinObj = pressio4pyGalerkin.ProblemRK4(appObj, yRef, decoder, yRom, t0, ops)
  stepper = galerkinObj.getStepper()
  pressio4pyGalerkin.integrateNStepsRK4(stepper, yRom, t0, dt, nsteps)


meshSize = int(sys.argv[1])
romSize  = int(sys.argv[2])
Nsteps   = int(sys.argv[3])
dt       = float(sys.argv[4])
np.set_printoptions(linewidth=400)
print(meshSize)
print(romSize)
print(Nsteps)
print(dt)

# create app
appObj = Burgers1d(meshSize)

# reference state
yRef = np.ones(meshSize)

ops = pressio4pyOps.Ops()
phi = np.loadtxt("basis.txt")
decoder = pressio4pyGalerkin.LinearDecoder(phi, ops)
yRom = np.zeros(romSize)

# do untimed warm up run for numba compilation
doGalerkinForTargetSteps(1, appObj, yRef, decoder, yRom, 0., ops)

# the actual timing starts here after the warm up
yRom *= 0
startTime = time.time()
doGalerkinForTargetSteps(Nsteps, appObj, yRef, decoder, yRom, 0., ops)
endTime = time.time()
elapsed = endTime-startTime
print("Elapsed time: {0:10.10f} ".format(elapsed) )

print ("Printing generalized coords to file")
np.savetxt("final_generalized_coords.txt", yRom, fmt='%.16f')
