#!/usr/bin/env python

import sys, os, time, re
from subprocess import Popen, list2cmdline, PIPE
import numpy as np
import os.path

def computeTimeOfSnapshot(snapId, samplingFreq, dt):
  # compute time of this snapId: since snapshots NEVER include the init cond,
  # snapId=0 corresponds to snapshot taken at step = samplingFreq, so to compute
  # the right time, we need to increment snapId by 1 before multiplying
  snapshotTime = (snapId+1) * samplingFreq * dt
  print( "sampling freq is = ", samplingFreq)
  print( "time = ", snapshotTime)
  return snapshotTime


def splitStateBySpecies(targetState, n):
  c0 = targetState[0:-1:3]
  c1 = targetState[1:-1:3]
  c2 = targetState[2:len(targetState)+1:3]
  c0rs = c0.reshape(n,n)
  c1rs = c1.reshape(n,n)
  c2rs = c2.reshape(n,n)
  return [c0,c1,c2,c0rs,c1rs,c2rs]


def extractFinalTimeFromTargetInputFile(dataDir):
  popen = Popen(["grep", "finalTime", str(dataDir)+"/input.txt"], stdout=PIPE)
  popen.wait()
  output = popen.stdout.read()
  finalTime = float(output.split()[1])
  print ("Inside " + dataDir + ", detected finalTime = ", finalTime)
  return finalTime


def extractSamplingFreqFromTargetInputFile(dataDir):
  # grab sampling frequency from the input file
  popen = Popen(["grep", "shapshotsFreq", str(dataDir)+"/input.txt"], stdout=PIPE)
  popen.wait()
  output = popen.stdout.read()
  samplingFrequency = int(output.split()[1])
  print ("Inside " + dataDir + ", detected samplingFrequncy = ", samplingFrequency)
  return samplingFrequency


def extractDtFromTargetInputFile(dataDir):
  # grab dt from the input file
  popen = Popen(["grep", "dt", str(dataDir)+"/input.txt"], stdout=PIPE)
  popen.wait()
  output = popen.stdout.read()
  dt = float(output.split()[1])
  print ("Inside " + dataDir + ", detected dt = ", dt)
  return dt
