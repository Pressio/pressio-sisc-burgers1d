
import numpy as np
import sys, time
import timeit
import math
from numba import jit, vectorize, njit, jitclass
from numba import f8, float64, int32
import profile
from scipy import linalg

from burgers1d import Burgers1dDenseJacobian
import pressio4pyGalerkin

def doGalerkinForTargetSteps(nsteps, appObj, yRef, decoder, yRom, t0):
  galerkinObj = pressio4pyGalerkin.ProblemRK4(appObj, yRef, decoder, yRom, t0)
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

# create app (for explicit Galerkin it does not matter the jacobian)
appObj = Burgers1dDenseJacobian(meshSize)
# reference state
yRef = np.ones(meshSize)
# load basis
phi = np.loadtxt("basis.txt")
decoder = pressio4pyGalerkin.LinearDecoder(phi)
yRom = np.zeros(romSize)

# do untimed warm up run for numba compilation
doGalerkinForTargetSteps(1, appObj, yRef, decoder, yRom, 0.)
# the actual timing starts here after the warm up
yRom *= 0
startTime = time.time()
doGalerkinForTargetSteps(Nsteps, appObj, yRef, decoder, yRom, 0.)
endTime = time.time()
elapsed = endTime-startTime
print("Elapsed time: {0:10.10f} ".format(elapsed) )
print ("Printing generalized coords to file")
np.savetxt("final_generalized_coords.txt", yRom, fmt='%.16f')



# ######################################
# f = np.ones(meshSize)
# yFom = np.ones(meshSize)
# yRom = np.ones(romSize)
# f = appObj.velocity(yFom, 0)
# phi = np.asfortranarray(phi)
# startTime = time.time()
# for i in range(5000):
#   #f = appObj.velocity(yFom, 0)
#   # phi^T f
#   linalg.blas.dgemv(1., phi, f, 0., yRom, 0, 1, 0, 1, 1, 1)
#   # phi yRom
#   #linalg.blas.dgemv(1., phi, yRom, 0., yFom, 0, 1, 0, 1, 0, 1)
#   decoder.applyMapping(yRom, yFom)
#   #appObj.velocity(yFom, 0., f)
#   #myVelFnc() #yRef, 0.0, f)
#   #yRom = np.dot(phi.T, yFom)
#   #decoder.phitvel(yFom, yRom)
#   #velocityImplNumba(yRef, 0.0, f, yTmp, 0.01, 5.)

# endTime = time.time()
# elapsed = endTime-startTime
# print("Elapsed time: {0:10.10f} ".format(elapsed) )
# #####################################################
