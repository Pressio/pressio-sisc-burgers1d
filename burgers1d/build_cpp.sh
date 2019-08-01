#!/bin/bash

# purpose: builds all exe for the C++ Burgers1d
# tpls needed: Eigen, Pressio
# assume the environemnt is already set,
#   i.e. CC,CXX are set

# set environment in some way, as long as CC,CXX are set
envScript=/Users/fnrizzi/Desktop/work/ROM/setenv_ompi400_clang700.sh
source ${envScript}
# TODO: test environment

# the working directory where everything will be put
WORKINGDIR=/Users/fnrizzi/Desktop/testFR


#--------------------------------------------
# Nothing should be changed below her
#--------------------------------------------
startDir=${PWD}

# test if workdir exists
if [ ! -d $WORKINGDIR ]; then
    echo "creating $WORKINGDIR" && mkdir -p $WORKINGDIR
fi
# go to working dir
cd ${WORKINGDIR}

# clone pressio
if [ ! -d pressio ]; then
    git clone --recursive git@gitlab.com:fnrizzi/pressio.git
    cd pressio && git checkout siscPaper && cd ..
else
    cd pressio && git pull && cd ..
fi

# clone pressio_auto_build
if [ ! -d pressio_auto_build ]; then
    git clone git@gitlab.com:fnrizzi/pressio_auto_build.git
    cd pressio_auto_build && git checkout siscPaper && cd ..
else
    cd pressio_auto_build && git pull && cd ..
fi

# get tpls (in this case only eigen)
# Note: does not matter dynamic/static since for Eigen nothing is compiled yet
if [ ! -d ${WORKINGDIR}/tpls/eigen ]; then
    cd pressio_auto_build/tpls
    bash main_tpls.sh -arch=mac \
	 --with-libraries=eigen \
	 -with-cmake-line-fncs=default \
	 --target-dir=${WORKINGDIR}/tpls \
	 --wipe-existing=1 \
	 -target-type=dynamic
    cd -
fi

# install pressio
# (do not build tests, just install with cmake which we need because of the cmakedefines)
# Note: does not matter dynamic/static/release/debug since nothing is compiled yet
if [ ! -d ${WORKINGDIR}/tpls/pressio ]; then
    cd pressio_auto_build/pressio
    bash main_pressio.sh -arch=mac \
	 -target-dir=${WORKINGDIR}/tpls \
	 -pressio-src=${WORKINGDIR}/pressio \
	 -all-tpls-path=${WORKINGDIR}/tpls \
	 -wipe-existing=1 \
	 -build-mode=Release \
	 -target-type=dynamic \
	 -with-cmake-fnc=sisc_paper_burgcpp \
	 -with-packages=rom
    cd -
fi

# set paths for eigen and pressio
EIGENPATH="${WORKINGDIR}/tpls/eigen/install/include/eigen3"
PRESSIOPATH="${WORKINGDIR}/tpls/pressio/install/include"

# build Burgers1d C++ exes
SRCDIR=${startDir}/cpp
bdirname=build
#check if build dir exists
[[ ! -d ${bdirname} ]] && mkdir ${bdirname}
# enter
cd ${bdirname}
cmake -DCMAKE_CXX_COMPILER=${CC} \
      -DCMAKE_CXX_COMPILER=${CXX} \
      -DCMAKE_VERBOSE_MAKEFILE:BOOL=TRUE \
      -DCMAKE_BUILD_TYPE:STRING=Release \
      -DEIGEN_INCLUDE_DIR=${EIGENPATH} \
      -DPRESSIO_INCLUDE_DIR=${PRESSIOPATH} \
      ${SRCDIR}
make -j4
cd ..

# go back where we started
cd ${startDir}







# n_args=$#
# if test $n_args -lt 1
# then
#     str+="[cmake-line-generator-fnc] "
#     # str+="[linux/mac] "
#     # str+="[dynamic/static] "

#     echo "usage:"
#     echo "$0 $str"
#     exit 1;
# fi
