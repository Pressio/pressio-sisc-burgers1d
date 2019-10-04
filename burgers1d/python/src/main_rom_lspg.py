
# by default, numpy has row-major memory layout
import numpy as np
from scipy import linalg
import sys, time

# local app class
from burgers1d import Burgers1d

# pressio bindings modules
import pressio4py
import pressio4pyLspg
import pressio4pyOps

np.set_printoptions(linewidth=400)

class MyLinSolver:
  def __init__(self, matrixType):
    self.assume_ = matrixType

  # default to LU-based solve
  def solve(self,A,b,x):
    x[:] = 1.0
    #x[:] = linalg.solve(A, b,
    #                    assume_a=self.assume_,
    #                    check_finite=False)


Ncell   = int(float(sys.argv[1]))
romSize = int(float(sys.argv[2]))
Nsteps  = int(float(sys.argv[3]))
dt      = float(sys.argv[4])

print(Ncell)
print(romSize)
print(Nsteps)
print(dt)

# create app
appObj = Burgers1d(Ncell)

# reference state
yRef = np.ones(Ncell)

# object in charge of ops
ops = pressio4pyOps.Ops()

# load basis into numpy array
phi0 = np.loadtxt("basis.txt")
phi = np.zeros((Ncell, romSize), order='F')
phi = phi0

# create a decoder
decoder = pressio4py.LinearDecoder(phi, ops)

# the LSPG (reduced) state
yRom = np.zeros(romSize)

# start timer
startTime = time.time()

# the problem
t0 = 0.
lspgObj = pressio4pyLspg.ProblemEuler(appObj, yRef, decoder, yRom, t0, ops)

# get stepper
stepper = lspgObj.getStepper()

# linear solver: this is used for the normal equations,
# which has a symmetric matrix, so use the information
lsO = MyLinSolver('gen')

# non linear solver
nlsO = pressio4pyLspg.GaussNewton(stepper, yRom, lsO, ops)
nlsO.setMaxIterations(10)
nlsO.setTolerance(1e-13)

pressio4pyLspg.integrateNSteps(stepper, yRom, 0.0, dt, Nsteps, nlsO)

endTime = time.time()
elapsed = endTime-startTime

print("Elapsed time: {0:10.10f} ".format(elapsed) )

print ("Printing first 5 elements of gen coords")
print (np.atleast_2d(yRom[:5]).T)
