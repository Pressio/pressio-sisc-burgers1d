
import numpy as np
import sys, time
import timeit
import math
from numba import jit, vectorize, njit, jitclass
from numba import f8, float64, int32
import profile

# from burgers1d import Burgers1d
import pressio4pyGalerkin
import pressio4pyOps
# def doGalerkinForTargetSteps(nsteps, appObj, yRef, decoder, yRom, t0, ops):
#   galerkinObj = pressio4pyGalerkin.ProblemRK4(appObj, yRef, decoder, yRom, t0, ops)
#   stepper = galerkinObj.getStepper()
#   pressio4pyGalerkin.integrateNStepsRK4(stepper, yRom, t0, dt, nsteps)

spec = [
  ('mu_', f8[:]), ('xL_', f8), ('xR_', f8), ('Ncell_', int32),
  ('dx_', f8), ('dxInvHalf_', f8),
  ('xGrid_', float64[::1]), ('U0_', float64[::1]),
  ('f_', float64[::1]), ('expVec_', float64[::1]),
]


def velocityVec(u, t, f, expVec, dxInvHalf, mu0):
  uSq_ = np.square(u)
  f[0]  = dxInvHalf * (mu0*mu0 - uSq_[0]) + expVec[0]
  f[1:] = dxInvHalf * ( uSq_[0:-1] - uSq_[1:] ) + expVec[1:]

@njit(["void(float64[::1], f8, float64[::1], float64[::1], f8, f8)"], fastmath=True)
def velocityImplNumba(u, t, f, expVec, dxInvHalf, mu0):
  f[0] = dxInvHalf * (mu0**2 - u[0]**2) + expVec[0]
  for i in range(1,len(u)):
    f[i] = dxInvHalf * (u[i-1]**2-u[i]**2) + expVec[i] # + math.sin(u[i])


#@jitclass(spec)
class Burgers1d:
  def __init__(self, Ncell):
    self.mu_    = np.array([5., 0.02, 0.02])
    self.xL_    = 0.
    self.xR_    = 100.
    self.Ncell_ = Ncell
    self.dx_    = 0.
    self.dxInvHalf_ = 0.
    self.xGrid_ = np.zeros(self.Ncell_)
    self.U0_    = np.zeros(self.Ncell_)
    self.f_     = np.zeros(self.Ncell_)
    self.expVec_= np.zeros(self.Ncell_)
    self.setup()

  def setup(self):
    self.dx_ = (self.xR_ - self.xL_)/float(self.Ncell_)
    self.dxInvHalf_ = 0.5 * (1.0/self.dx_)
    for i in range(0, self.Ncell_):
      self.U0_[i] = 1.
      self.xGrid_[i] = self.dx_*i + self.dx_*0.5
    self.expVec_ = self.mu_[1] * np.exp( self.mu_[2] * self.xGrid_ )

  # def velocity(self, *args):
  #   u, t = args[0], args[1]
  #   if len(args) == 2:
  #     velocityImplNumba(u, t, self.f_, self.expVec_, self.dxInv_, self.mu_[0])
  #     return self.f_
  #   else:
  #     velocityImplNumba(u, t, args[2], self.expVec_, self.dxInv_, self.mu_[0])

  def velocityEmpty(self, u, t, f):
    pass

  def velocity(self, u, t, f):
   # dxInvHf = self.dxInvHalf_
   # expV = self.expVec_
   # f[0] = dxInvHf * (self.mu_[0]**2 - u[0]**2) + expV[0]
   # for i in range(1,len(u)):
   #   f[i] = dxInvHf * (u[i-1]**2-u[i]**2) + expV[i] # + math.sin(u[i])

    # coeff = 0.5*self.dxInv_
    # mu0 = self.mu_[0]
    # f[0] = coeff * (mu0*mu0 - u[0]*u[0])
    # for i in range(1,len(u)):
    #   f[i] = coeff * ( u[i-1]*u[i-1] - u[i]*u[i] )
    # #for i in range(len(u)):
    # #  f[i] += self.expVec_[i]
    velocityImplNumba(u, t, f, self.expVec_, self.dxInvHalf_, self.mu_[0])


def runFromObj1(appObj, yRef, f, nRounds):
  myVelFnc = appObj.velocity
  for i in range(nRounds):
    myVelFnc(yRef, 0.0, f)

def runFromObj2(appObj, yRef, f, nRounds):
  for i in range(nRounds):
    appObj.velocity(yRef, 0.0, f)

def runFromFreeFunc(appObj, yRef, f, nRounds):
  veloTest(yRef, 0.0, f)


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

# reference state
yRef = np.ones(meshSize)
yTmp = np.ones(meshSize)
f = np.ones(meshSize)
# # call velocity once to let numba compile code
# # so that below we don't measure the overhead of compilation
#appObj.velocity(yRef, 0.0, f)
#velocityImplNumba(yRef, 0.0, f, yTmp, 0.01, 5.)
#veloTest(yRef, 0.0, f, yTmp, 0.01, 5.)
#warmup
#runFromObj2(appObj, yRef, f, 10)

ops = pressio4pyOps.Ops()
phi = np.loadtxt("basis.txt")
#phi = np.asfortranarray(phi0)
#phi = np.zeros((meshSize, romSize))
decoder = pressio4pyGalerkin.LinearDecoder(phi, ops)
yRom = np.ones(romSize)

########################################
yFom = np.ones(meshSize)
nRounds=1
#myVelFnc = appObj.velocityEmpty
#myVelFnc(yRef, 0.0, f)

startTime = time.time()
#runFromObj2(appObj, yRef, f, nRounds)
for i in range(nRounds):
  decoder.applyMapping(yRom, yFom)

  #appObj.velocity(yFom, 0., f)
  #myVelFnc() #yRef, 0.0, f)
  #yRom = np.dot(phi.T, yFom)
  #decoder.phitvel(yFom, yRom)
  #velocityImplNumba(yRef, 0.0, f, yTmp, 0.01, 5.)


endTime = time.time()
elapsed = endTime-startTime
print("Elapsed time: {0:10.10f} ".format(elapsed) )
print("timer per iter: {0:10.10f} ".format(elapsed/nRounds) )

np.savetxt("yFomReconstructed.txt", yFom, fmt='%.16f')
########################################

# # do untimed warm up run for numba compilation
# doGalerkinForTargetSteps(1, appObj, yRef, decoder, yRom, 0., ops)

# # the actual timing starts here after the warm up
# yRom *= 0
# startTime = time.time()
# doGalerkinForTargetSteps(Nsteps, appObj, yRef, decoder, yRom, 0., ops)
# endTime = time.time()
# elapsed = endTime-startTime
# print("Elapsed time: {0:10.10f} ".format(elapsed) )

# print ("Printing generalized coords to file")
# np.savetxt("final_generalized_coords.txt", yRom, fmt='%.16f')











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
