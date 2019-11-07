import numpy as np
from numba import jit
from scipy.sparse import csr_matrix, diags, spdiags
import time

# by default, numpy has row-major memory layout

#@jit(cache=True)
@jit(nopython=True)
def velocityImplNumba(u, t, f, expVec_, dxInv_, mu0):
  f[0] = 0.5 * dxInv_ * (mu0*mu0 - u[0]*u[0])
  for i in range(1,len(u)):
    f[i] = 0.5 * dxInv_ * ( u[i-1]*u[i-1] - u[i]*u[i] )
  for i in range(len(u)):
    f[i] += expVec_[i]

# @jit(cache=True)
# def velocityImplNumba1(u, t, f, expVec_, dxInv_, mu0):
#   #f[:] += expVec_[:] * np.sin(u[:])
#   n = len(u)
#   for i in range(n):
#     f[i] += expVec_[i] * np.sin(u[i])

@jit(nopython=True)
def jacobianImplNumba(u, t, J, dxInv, N):
  J[0][0] = -dxInv*u[0]
  for i in range(1, N):
    J[i][i-1] = dxInv * u[i-1]
    J[i][i] = -dxInv * u[i]

@jit(nopython=True)
def fillDiag(u, diag, ldiag, dxInv):
  for i in range(len(u)):
    diag[i] = -dxInv*u[i]
    if i < len(u)-1:
      ldiag[i] = dxInv*u[i]

class Burgers1d:
  def __init__(self, Ncell):
    self.mu_    = np.array([5., 0.02, 0.02])
    self.xL_    = 0.
    self.xR_    = 100.
    self.Ncell_ = Ncell
    self.dx_    = 0.
    self.dxInv_ = 0.
    self.xGrid_ = np.zeros(self.Ncell_)
    self.U0_    = np.zeros(self.Ncell_)
    self.f_     = np.zeros(self.Ncell_)
    self.expVec_= np.zeros(self.Ncell_)
    self.diag   = np.zeros(self.Ncell_)
    self.ldiag  = np.zeros(self.Ncell_-1)
    self.J_     = np.zeros((self.Ncell_, self.Ncell_))
    # call the setup
    self.setup()

  def setup(self):
    self.dx_ = (self.xR_ - self.xL_)/float(self.Ncell_)
    self.dxInv_ = 1.0/self.dx_;
    self.U0_[:] = 1.
    for i in range(0, self.Ncell_):
      self.xGrid_[i] = self.dx_*i + self.dx_*0.5
    self.expVec_ = self.mu_[1] * np.exp( self.mu_[2] * self.xGrid_ )

  def velocity(self, *args):
    u, t = args[0], args[1]
    if len(args) == 2:
      velocityImplNumba(u, t, self.f_, self.expVec_, self.dxInv_, self.mu_[0])
      return self.f_
    else:
      velocityImplNumba(u, t, self.f_, self.expVec_, self.dxInv_, self.mu_[0])
      args[2][:] = self.f_

  # ###### for sparse Jacobian ######
  # def jacobian(self, u, t):
  #   #self.diag[:] = -self.dxInv_*u[:]
  #   #self.ldiag[:] = self.dxInv_*u[0:-1]
  #   fillDiag(u, self.diag, self.ldiag, self.dxInv_)
  #   return diags( [self.ldiag, self.diag], [-1,0])

  # def applyJacobian(self, *args):
  #   # A = J*B, with J evaluated for given x,t
  #   u, B, t = args[0], args[1], args[2]
  #   if len(args) == 3:
  #     return self.jacobian(u, t).dot(B)
  #   else:
  #     A = args[3]
  #     A[:] = self.jacobian(u, t).dot(B)

  ###### for dense Jacobian ######
  def jacobian(self, *args):
    u, t = args[0], args[1]
    if len(args) == 2:
      jacobianImplNumba(u, t, self.J_, self.dxInv_, self.Ncell_)
      return self.J_
    else:
      jacobianImplNumba(u, t, args[2], self.dxInv_, self.Ncell_)

  def applyJacobian(self, *args):
    # A = J*B, with J evaluated for given x,t
    u, B, t = args[0], args[1], args[2]
    if len(args) == 3:
      J = self.jacobian(u, t)
      return np.matmul(J,B)
    else:
      A = args[3]
      J = self.jacobian(u, t)
      A[:] = np.matmul(J,B)




  # def jacobianImpl(self, u, t):
  #   #self.diag[:] = -self.dxInv_*u[:]
  #   #self.ldiag[:] = self.dxInv_*u[0:-1]
  #   fillDiag( u, self.diag, self.ldiag, self.dxInv_)
  #   return diags( [self.ldiag, self.diag], [-1,0])

  # def jacobian(self, *args):
  #   u, t = args[0], args[1]
  #   if len(args) == 2:
  #     return jacobianImplNumba(u, t, self.dxInv_,
  #                              self.Ncell_, self.diag, self.ldiag)
  #   else:
  #     args[2][:] = jacobianImplNumba(u, t, self.dxInv_,
  #                                    self.Ncell_, self.diag, self.ldiag)

  # def jacobianImpl(self, u, t):
  #   self.diag[:] = -self.dxInv_*u[:]
  #   self.ldiag[:] = self.dxInv_*u[0:-1]
  #   return diags( [self.ldiag, self.diag], [-1,0],
  #                 shape=[self.Ncell_, self.Ncell_],
  #                 format='csr')


# @jit(nopython=True)
# def velocityImpl(self, u, t, f):
#   #startTime = time.time()
#   uSq_ = u #np.square(u)
#   f[0] = 0.5 * self.dxInv_ * (self.mu0sq_ - uSq_[0])
#   f[1:] = 0.5 * self.dxInv_ * ( uSq_[0:-1] - uSq_[1:] )
#   f[:] += self.expVec_
#   # endTime = time.time()
#   # elapsed = endTime-startTime
#   # print("f-time: {0:10.10f} ".format(elapsed) )

  # def jacobianImpl(self, u, t):
  #   self.J_[np.arange(self.Ncell_), np.arange(self.Ncell_)] = -self.dxInv_*u[:]
  #   self.J_[np.arange(1, self.Ncell_),
  #           np.arange(self.Ncell_-1)] = self.dxInv_*u[0:self.Ncell_-1]

  #   # self.J_[0][0] = -self.dxInv_*u[0]
  #   # for i in range(self.Ncell_):
  #   #     self.J_[i][i-1] = self.dxInv_ * u[i-1]
  #   #     self.J_[i][i] = -self.dxInv_ * u[i]
  #   return self.J_


# @jit(nopython=True)
# def velocityImplVectorize(u, t, f, expVec_, dxInv_, mu0):
#   uSq_ = np.square(u)
#   f[0] = 0.5 * dxInv_ * (mu0*mu0 - uSq_[0])
#   f[1:] = 0.5 * dxInv_ * ( uSq_[0:-1] - uSq_[1:] )
#   f[:] += expVec_









#########################################



# def jacobianImplVectorize(u, t, J, dxInv, N):
#   J[np.arange(N), np.arange(N)] = -dxInv*u[:]
#   J[np.arange(1, N), np.arange(N-1)] = dxInv*u[0:N-1]

# #   #J[0][0] = -self.dxInv_*u[0]
# #   # for i in range(self.Ncell_):
# #   #     self.J_[i][i-1] = self.dxInv_ * u[i-1]
# #   #     self.J_[i][i] = -self.dxInv_ * u[i]
# #   for i in range(N):
# #     diag[i] = -dxInv*u[i]
# #     if i < N: ldiag[i] = dxInv*u[i]
# #   return diags( [ldiag, diag], [-1,0], shape=[N, N], format='csr')
