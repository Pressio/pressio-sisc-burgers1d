
# by default, numpy has row-major memory layout
import numpy as np
import sys, time
from scipy.sparse import csr_matrix, diags, spdiags

np.set_printoptions(linewidth=400)

nArows = int(sys.argv[1])
nAcols = int(sys.argv[2])
nBrows = int(sys.argv[3])
nBcols = int(sys.argv[4])
nReplic = int(sys.argv[5])

print("A dims = ", nArows, nAcols)
print("B dims = ", nBrows, nBcols)
print(nReplic)

Vdm1 = 343.232;
Vd   = 34.1232;
Vdp1 = 5654.55652;
ldiag = Vdm1*np.ones(nAcols-1)
diag  = Vd*np.ones(nArows)
udiag = Vdp1*np.ones(nAcols-1)

A = diags( [ldiag, diag, udiag], [-1,0,1],
           shape=[nArows, nAcols],
           format='csr')

#print(A.todense())
#B0 = np.random.rand(nBrows, nBcols)
B = np.ones((nBrows, nBcols), order='C')
#B = B0

C = np.zeros((nArows, nBcols), order='C')

# start timer
startTime = time.time()

for i in range(nReplic+1):
  C = A.dot(B)

endTime = time.time()
elapsed = endTime-startTime
eT_avg = elapsed/nReplic

print("Elapsed time: {0:10.10f} ".format(elapsed) )
print("Avg time per run: {0:10.10f} ".format(eT_avg) )

gflop = 0.0
print("GFlopPerItem: {0:10.10f}".format(gflop) )
print("GFlop/sec: {0:10.10f} ".format(gflop/eT_avg) )
