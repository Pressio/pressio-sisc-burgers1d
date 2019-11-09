import numpy as np
from numba import jit, njit
from scipy.sparse import csr_matrix, diags, spdiags
import time


@njit(["void(float64[::1], f8, float64[::1], float64[::1], f8, f8)"], fastmath=True)
def velocityImplNumba(u, t, f, expVec, dxInvHalf, mu0):
  f[0] = dxInvHalf * (mu0**2 - u[0]**2) + expVec[0]
  for i in range(1,len(u)):
    f[i] = dxInvHalf * (u[i-1]**2-u[i]**2) + expVec[i]

@njit(["void(float64[::1], f8, float64[::1, :], f8, int32)"], fastmath=True)
def jacobianImplNumba(u, t, J, dxInv, N):
  J[N-1][N-1] = -dxInv*u[N-1]
  for i in range(0, N-1):
    J[i][i] = -dxInv * u[i]
    J[i+1][i] = dxInv * u[i]

@njit(["void(float64[:], float64[:], float64[:], f8)"], fastmath=True)
def fillDiag(u, diag, ldiag, dxInv):
  n = len(u)
  for i in range(n-1):
    diag[i] = -dxInv*u[i]
    ldiag[i] = dxInv*u[i]
  diag[n-1] = -dxInv*u[n-1]



class Burgers1d:
  def __init__(self, Ncell, useDense=True):
    self.dense_ = useDense
    self.mu_    = np.array([5., 0.02, 0.02])
    self.xL_    = 0.
    self.xR_    = 100.
    self.Ncell_ = Ncell
    self.dx_    = 0.
    self.dxInv_ = 0.
    self.dxInvHalf_ = 0.
    self.xGrid_ = np.zeros(self.Ncell_)
    self.U0_    = np.zeros(self.Ncell_)
    self.f_     = np.zeros(self.Ncell_)
    self.expVec_= np.zeros(self.Ncell_)
    self.diag_  = np.zeros(self.Ncell_)
    self.ldiag_ = np.zeros(self.Ncell_-1)
    if useDense:
      self.J_   = np.zeros((self.Ncell_, self.Ncell_), order='F')
    self.setup()

  def setup(self):
    self.dx_ = (self.xR_ - self.xL_)/float(self.Ncell_)
    self.dxInv_ = (1.0/self.dx_)
    self.dxInvHalf_ = 0.5 * self.dxInv_
    for i in range(0, self.Ncell_):
      self.U0_[i] = 1.
      self.xGrid_[i] = self.dx_*i + self.dx_*0.5
    self.expVec_ = self.mu_[1] * np.exp( self.mu_[2] * self.xGrid_ )

  def velocity(self, *args):
    u, t = args[0], args[1]
    if len(args) == 2:
      velocityImplNumba(u, t, self.f_, self.expVec_, self.dxInvHalf_, self.mu_[0])
      return self.f_
    else:
      velocityImplNumba(u, t, args[2], self.expVec_, self.dxInvHalf_, self.mu_[0])

  def jacobian(self, *args):
    if len(args) == 2:
      if self.dense_:
        return self.jacobianDense(args)
      else:
        return self.jacobianSparse(args)
    else:
      if self.dense_:
        self.jacobianDense(args)
      else:
        self.jacobianSparse(args)

  def applyJacobian(self, *args):
    if len(args) == 3:
      if self.dense_:
        return self.applyJacobianDense(args[0], args[1], args[2])
      else:
        return self.applyJacobianSparse(args[0], args[1], args[2])
    else:
      if self.dense_:
        self.applyJacobianDense(args[0], args[1], args[2], args[3])
      else:
        self.applyJacobianSparse(args[0], args[1], args[2], args[3])

  ##################################
  ###### for sparse Jacobian ######
  def jacobianSparse(self, u, t):
    #self.diag_[:] = -self.dxInv_*u[:]
    #self.ldiag_[:] = self.dxInv_*u[0:-1]
    fillDiag(u, self.diag_, self.ldiag_, self.dxInv_)
    return diags( [self.ldiag_, self.diag_], [-1,0], format='coo')

  def applyJacobianSparse(self, *args):
    # A = J*B, with J evaluated for given x,t
    u, B, t = args[0], args[1], args[2]
    if len(args) == 3:
      return self.jacobianSparse(u, t).dot(B)
    else:
      A = args[3]
      A[:] = self.jacobianSparse(u, t).dot(B)

  ##################################
  ###### for dense Jacobian ######
  def jacobianDense(self, *args):
    u, t = args[0], args[1]
    if len(args) == 2:
      jacobianImplNumba(u, t, self.J_, self.dxInv_, self.Ncell_)
      return self.J_
    else:
      jacobianImplNumba(u, t, args[2], self.dxInv_, self.Ncell_)

  def applyJacobianDense(self, *args):
    # A = J*B, with J evaluated for given x,t
    u, B, t = args[0], args[1], args[2]
    if len(args) == 3:
      self.jacobianDense(u, t, self.J_)
      return np.matmul(self.J_,B)
    else:
      A = args[3]
      self.jacobianDense(u, t, self.J_)
      A[:] = np.matmul(self.J_,B)
  ##################################
