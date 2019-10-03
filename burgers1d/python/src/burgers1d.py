import numpy as np
from scipy.sparse import csr_matrix, diags

# by default, numpy has row-major memory layout

class Burgers1d:
  def __init__(self, Ncell):
    self.mu_ = np.array([5., 0.02, 0.02])
    self.xL_ = 0.0
    self.xR_ = 100.
    self.Ncell_ = Ncell
    self.dx_ = 0.
    self.dxInv_ = 0.
    self.xGrid_ = np.zeros(self.Ncell_)
    self.U0_ = np.zeros(self.Ncell_)
    self.setup()
    self.f_ = np.zeros(self.Ncell_)
    self.diag = np.zeros(self.Ncell_)
    self.ldiag = np.zeros(self.Ncell_-1)

  def setup(self):
    self.dx_ = (self.xR_ - self.xL_)/float(self.Ncell_)
    self.dxInv_ = 1.0/self.dx_;
    self.U0_[:] = 1.
    for i in range(0, self.Ncell_):
      self.xGrid_[i] = self.dx_*i + self.dx_*0.5

  def velocity(self, *args):
    u, t = args[0], args[1]
    if len(args) == 2:
      self.velocityImpl(u, t, self.f_)
      return self.f_
    else:
      f = args[2]
      self.velocityImpl(u, t, f)

  def velocityImpl(self, u, t, f):
    f[0] = 0.5 * self.dxInv_ * (self.mu_[0]**2 - u[0]**2)
    f[1:] = 0.5 * self.dxInv_ * (u[0:-1]**2 - u[1:]**2)
    f[:] += self.mu_[1] * np.exp( self.mu_[2] * self.xGrid_[:] )

  def jacobianImpl(self, u, t):
    self.diag[:] = -self.dxInv_*u[:]
    self.ldiag[:] = self.dxInv_*u[0:-1]
    return diags( [self.ldiag, self.diag], [-1,0],
                  shape=[self.Ncell_, self.Ncell_], format='csr')

  def jacobian(self, *args):
    u, t = args[0], args[1]
    if len(args) == 2:
      return self.jacobianImpl(u, t)
    else:
      args[2] = self.jacobianImpl(u, t)

  def applyJacobian(self, *args):
    # A = J*B, with J evaluated for given x,t
    u, B, t = args[0], args[1], args[2]
    if len(args) == 3:
      J = self.jacobian(u, t)
      return J.dot(B)
    else:
      A = args[3]
      J = self.jacobian(u, t)
      A[:] = J.dot(B)
