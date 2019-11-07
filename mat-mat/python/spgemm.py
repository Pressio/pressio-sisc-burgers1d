
# by default, numpy has row-major memory layout
import numpy as np
import sys, time
from scipy.sparse import csr_matrix, diags, spdiags
import timeit

np.set_printoptions(linewidth=400)
np.show_config()

numDofs = int(sys.argv[1])
romSize = int(sys.argv[2])
nReplic = int(sys.argv[3])

print("n dofs = ", numDofs)
print("rom size = ", romSize)
print(nReplic)

rA = numDofs;
cA = rA;
rB = cA;
cB = romSize;

Vdm1 = 343.232;
Vd   = 34.1232;
Vdp1 = 5654.55652;
ldiag = Vdm1*np.ones(cA-1)
diag  = Vd*np.ones(rA)
udiag = Vdp1*np.ones(cA-1)

A = diags( [ldiag, diag, udiag],
           [-1,0,1],
           shape=[rA, cA],
           format='csr')

B = np.ones((rB, cB), order='C')
C = np.zeros((rA, cB), order='C')

startTime = time.time()

for i in range(nReplic+1):
  C[:] = A.dot(B)


endTime = time.time()
elapsed = endTime-startTime
eT_avg = elapsed/nReplic

print("Elapsed time: {0:10.10f} ".format(elapsed) )
print("Avg time per run: {0:10.10f} ".format(eT_avg) )

gflop = 0.0
print("GFlopPerItem: {0:10.10f}".format(gflop) )
print("GFlop/sec: {0:10.10f} ".format(gflop/eT_avg) )
