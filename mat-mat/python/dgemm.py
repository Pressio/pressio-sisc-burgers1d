
# by default, numpy has row-major memory layout
import numpy as np
import sys, time

np.set_printoptions(linewidth=400)

nArows = int(sys.argv[1])
nAcols = int(sys.argv[2])
nBrows = int(sys.argv[3])
nBcols = int(sys.argv[4])
nReplic = int(sys.argv[5])

print("A dims = ", nArows, nAcols)
print("B dims = ", nBrows, nBcols)
print(nReplic)

#A0 = np.random.rand(nArows, nAcols)
A = np.ones((nArows, nAcols), order='C')
#A = A0

#B0 = np.random.rand(nBrows, nBcols)
B = 2*np.ones((nBrows, nBcols), order='C')
#B = B0

C = np.zeros((nArows, nBcols), order='C')

# start timer
startTime = time.time()

for i in range(nReplic+1):
  C = np.dot(A.T, B)

endTime = time.time()
elapsed = endTime-startTime
eT_avg = elapsed/nReplic

print("Elapsed time: {0:10.10f} ".format(elapsed) )
print("Avg time per run: {0:10.10f} ".format(eT_avg) )

gflop = (2. * nArows * nAcols * nBcols)*1e-9
print("GFlopPerItem: {0:10.10f}".format(gflop) )
print("GFlop/sec: {0:10.10f} ".format(gflop/eT_avg) )
