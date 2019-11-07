
# by default, numpy has row-major memory layout
import numpy as np
import sys, time
import scipy.linalg.blas as scblas
from numba import jit

np.set_printoptions(linewidth=400)


# def doProd(A,B,C):
#   for i in range(nReplic+1):
#     C[:] = np.dot(A.T, B)

# def doVec(f, f2):
#   for i in range(nReplic+1):
#     fSq = np.square(f2)
#     f[0] = 0.5 * 0.001 * (5.*5 - fSq[0])
#     f[1:] = 0.5 * 0.001 * ( fSq[0:-1] - fSq[1:] )
#scblas.dgemm(1.0, A, B, 0.0, C, 1, 0, 1)
#C[:] = np.dot(A.T, B)

@jit(nopython=True)
def myF(f, f2):
  for i in range(nReplic+1):
    f += 0.005 * np.exp(2.) * np.sin(3.14)
    f += f2


numDofs = int(sys.argv[1])
romSize = int(sys.argv[2])
nReplic = int(sys.argv[3])

print("n dofs = ", numDofs)
print("rom size = ", romSize)
print(nReplic)

np.show_config()

#A0 = np.random.rand(nArows, nAcols)
A = np.ones((numDofs, numDofs), order='C')
B = 2*np.ones((numDofs, romSize), order='C')
C = np.zeros((numDofs, romSize), order='C')
f = np.zeros(numDofs)
f2 = 4*np.ones(numDofs)

myF(f, f2)
startTime = time.time()
myF(f,f2)

endTime = time.time()
elapsed = endTime-startTime
eT_avg = elapsed/nReplic

print("Elapsed time: {0:10.10f} ".format(elapsed) )
print("Avg time per run: {0:10.10f} ".format(eT_avg) )

gflop = 0.0 #(2. * numDofs * numDofs * romSize)*1e-9
print("GFlopPerItem: {0:10.10f}".format(gflop) )
print("GFlop/sec: {0:10.10f} ".format(gflop/eT_avg) )
