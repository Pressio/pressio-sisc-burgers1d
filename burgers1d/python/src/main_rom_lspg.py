
# by default, numpy has row-major memory layout
import numpy as np
from scipy import linalg
from numba import jit, njit
import sys, time
# local app class
from burgers1d import Burgers1d
# pressio bindings modules
import pressio4pyLspg
import pressio4pyOps

np.set_printoptions(linewidth=400)


@njit(["void(float64[::1], float64[::1])"], fastmath=True)
def mycopy(src, dest):
  n = len(src)
  for i in range(n):
    dest[i] = src[i]


class MyLinSolver:
  def __init__(self):
    pass

  @staticmethod
  def solve(A,b,x):
    lumat, piv, info = linalg.lapack.dgetrf(A, overwrite_a=True)
    x[:], info = linalg.lapack.dgetrs(lumat, piv, b, 0, 0)


def doLSPGForTargetSteps(nsteps, appObj, yRef, decoder, yRom, t0, ops, nlsMaxIt, nlsTol):
  lspgObj = pressio4pyLspg.ProblemEuler(appObj, yRef, decoder, yRom, t0, ops)
  stepper = lspgObj.getStepper()
  # pass sym since matrix for NEq is symmetric
  lsO = MyLinSolver()
  # non linear solver
  nlsO = pressio4pyLspg.GaussNewton(stepper, yRom, lsO, ops)
  nlsO.setMaxIterations(nlsMaxIt)
  nlsO.setTolerance(nlsTol)
  # do integration
  pressio4pyLspg.integrateNSteps(stepper, yRom, t0, dt, nsteps, nlsO)


###########################################
###########################################
Ncell   = int(sys.argv[1])
romSize = int(sys.argv[2])
Nsteps  = int(sys.argv[3])
dt      = float(sys.argv[4])
print(Ncell)
print(romSize)
print(Nsteps)
print(dt)

# create app
appObj = Burgers1d(Ncell)

# set reference state
yRef = np.ones(Ncell)

# object in charge of ops
ops = pressio4pyOps.Ops()

# load basis into numpy array
phi = np.loadtxt("basis.txt")

# create a decoder
decoder = pressio4pyLspg.LinearDecoder(phi, ops)
# the LSPG (reduced) state
yRom = np.zeros(romSize)

nlsTol, nlsMaxIt = 1e-13, 20
t0 = 0.

# do untimed warm up run for numba compilation
doLSPGForTargetSteps(1, appObj, yRef, decoder, yRom, t0, ops, nlsMaxIt, nlsTol)

# the actual timing starts here after the warm up
yRom *= 0
startTime = time.time()
doLSPGForTargetSteps(Nsteps, appObj, yRef, decoder, yRom, t0, ops, nlsMaxIt, nlsTol)
endTime = time.time()
elapsed = endTime-startTime
print("Elapsed time: {0:10.10f} ".format(elapsed) )
print ("Printing generalized coords to file")
np.savetxt("final_generalized_coords.txt", yRom, fmt='%.15f')




# ############################################
# lsO = MyLinSolver('sym')
# startTime = time.time()
# yFom = np.ones(Ncell)
# yr = np.zeros(romSize)
# for i in range(1000):
#   f = appObj.velocity(yFom, 0)
#   J = appObj.applyJacobian(yFom, phi, 0)
#   jtj = np.dot(J.T, J)
#   jtf = np.dot(J.T, f)
#   lsO.solve(jtj, jtf, yr)
#   #ja = appObj.applyJacobian(yFom, phi, 0.0)
#   #ops.multiply2(phi, yFom, yRom, True)
#   #decoder.applyMapping(yRom, yRef)

# endTime = time.time()
# elapsed = endTime-startTime
# print("Elapsed time: {0:10.10f} ".format(elapsed) )
# ############################################
