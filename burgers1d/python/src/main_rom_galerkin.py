
import numpy as np
import sys, time
from numba import jit

# local app class
from burgers1d import Burgers1d
# pressio bindings modules
import pressio4py
import pressio4pyGalerkin
import pressio4pyOps

@jit(nopython=True)
def doUpdate(v, a, v2, b):
  for i in range(0,len(v)):
    v[i] = 0.5 * v[i] + b * v2[i]


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
t0 = 0.

# reference state
yRef = np.ones(meshSize)
# call velocity once to let numba compile code
# so that below we don't measure the overhead of compilation
f = appObj.velocity(yRef, 0.0)

# object in charge of ops
ops = pressio4pyOps.Ops()

# load basis into numpy array
phi0 = np.loadtxt("basis.txt")
phi = np.asfortranarray(phi0)
# create a decoder
decoder = pressio4py.LinearDecoder(phi, ops)

# the LSPG (reduced) state
yRom = np.zeros(romSize)

# start timer
startTime = time.time()
galerkinObj = pressio4pyGalerkin.ProblemRK4(appObj, yRef, decoder, yRom, t0, ops)
stepper = galerkinObj.getStepper()
pressio4pyGalerkin.integrateNStepsRK4(stepper, yRom, t0, dt, Nsteps)

endTime = time.time()
elapsed = endTime-startTime
print("Elapsed time: {0:10.10f} ".format(elapsed) )

print ("Printing generalized coords to file")
np.savetxt("final_generalized_coords.txt", yRom, fmt='%.16f')









# ########################################
# yFom = np.zeros(meshSize)
# startTime = time.time()
# for i in range(1000):
#   decoder.test(yRom, yFom)
# endTime = time.time()
# elapsed = endTime-startTime
# print("Elapsed time: {0:10.10f} ".format(elapsed) )
# ########################################

# ########################################
# yFom  = np.ones(meshSize)
# yFom2 = np.ones(meshSize)
# doUpdate(yFom, 1., yFom2, dt)
# startTime = time.time()
# for i in range(1000):
#   #doUpdate(yFom, 1., yFom2, dt)
#   #yFom = 1.0*yFom + dt*yFom2
#   pressio4pyGalerkin.doUpdate(yFom, 1.0, yFom2, dt)
# endTime = time.time()
# elapsed = endTime-startTime
# print("Elapsed time: {0:10.10f} ".format(elapsed) )
# ########################################
