#!/bin/bash

# what does script do: builds all exe for the C++ Burgers1d
# tpls needed: Eigen, Pressio

startDir=${PWD}

# set environment script
envScript=/Users/fnrizzi/Desktop/work/ROM/setenv_ompi400_clang700.sh
source ${envScript}

# the working directory where everything will be put
WORKINGDIR=

# go to working dir
cd ${WORKINGDIR}

# clone pressio
git clone --recursive git@gitlab.com:fnrizzi/pressio.git
cd pressio && git checkout siscPaper && cd ..

# clone pressio_auto_build
git clone git@gitlab.com:fnrizzi/pressio_auto_build.git
cd pressio_auto_build && git checkout siscPaper && cd ..

# clone/build tpls
# install pressio (do not build tests, just install library with cmake
# which we need because of the cmakedefines)

# set paths for eigen and pressio
EIGENPATH="${WORKINGDIR}/eigen/install/include/eigen3"
PRESSIOPATH="${WORKINGDIR}/pressio/install/include"

# build Burgers1d C++ exes
SRCDIR=${startDir}
bdirname=build
rm -rf ${bdirname} && mkdir ${bdirname} && cd ${bdirname}
cmake -DCMAKE_CXX_COMPILER=${CC} \
      -DCMAKE_CXX_COMPILER=${CXX} \
      -DCMAKE_BUILD_TYPE:STRING=Release \
      -DEIGEN_INCLUDE_DIR=${EIGENPATH} \
      -DPRESSIO_INCLUDE_DIR=${PRESSIOPATH} \
      ${SRCDIR}
make
cd ..

# go back where we started
cd ${startDir}
