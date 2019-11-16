
import numpy as np
import sys, time
import scipy.linalg.blas as scblas
# local app class
from burgers1d import Burgers1d

meshSize = int(sys.argv[1])

np.set_printoptions(linewidth=400)
print(meshSize)
# create app
appObj = Burgers1d(meshSize)
yRef = np.ones(meshSize)
t0 = 0.0

romSize = 100
numRound = 5000

# phi_C means that phi_C.T will be col major
phi_C = np.ones((meshSize, romSize), order = 'C')
print (phi_C.flags)
phi_F = np.ones((meshSize, romSize), order = 'F')
print (phi_F.flags)

phi = phi_F
a = np.zeros(romSize)
a1 = np.zeros(meshSize)

#--- one run for warmup ---
f = appObj.velocity(yRef, t0)
a = np.dot(phi.T, f)
a1 = np.dot(phi, a)

#scblas.dgemv(1.0, phi, f, 0.0, A, 0, 1, 0, 1, 1, 1)
# J = np.zeros((meshSize, meshSize))
# appObj.jacobian(yRef, t0, J)

#--- do many replicas ---
startTime = time.time()
for i in range(numRound):
  appObj.velocity(yRef, t0, f)
  #scblas.dgemv(1.0, phi, f, 0.0, A, 0, 1, 0, 1, 1, 1)
  a  = np.dot(phi.T, f)
  a1 = np.dot(phi, a)

endTime = time.time()
elapsed = endTime-startTime
print("Elapsed time: {0:10.10f} ".format(elapsed) )
print("Time per round: {0:10.10f} ".format(elapsed/numRound) )
