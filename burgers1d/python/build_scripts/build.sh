#!/bin/bash

# purpose: builds pressio4py for running Python Burgers1d

# go to working dir
cd ${PYWORKINGDIR}

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

# clone pressio4py
if [ ! -d pressio4py ]; then
    git clone git@gitlab.com:fnrizzi/pressio4py.git
    cd pressio4py && git checkout siscPaper && cd ..
else
    cd pressio4py && git pull && cd ..
fi


# get tpls (in this case only eigen,pybind11)
# Note: does not matter dynamic/static since nothing is compiled yet
if [ ! -d ${PYWORKINGDIR}/tpls/pybind11 ]; then
    cd pressio_auto_build/tpls
    bash main_tpls.sh -arch=mac \
	 --with-libraries=eigen,pybind11 \
	 -with-cmake-line-fncs=default,default \
	 --target-dir=${PYWORKINGDIR}/tpls \
	 --wipe-existing=1 \
	 -build-mode=Release \
	 -target-type=dynamic
    cd ${PYWORKINGDIR}
fi

# install pressio
# (no tests, just install with cmake, needed because of the cmakedefines)
# Note: does not matter dynamic/static/release/debug since nothing is compiled yet
if [ ! -d ${PYWORKINGDIR}/tpls/pressio ]; then
    cd pressio_auto_build/pressio
    bash main_pressio.sh -arch=mac \
	 -target-dir=${PYWORKINGDIR}/tpls \
	 -pressio-src=${PYWORKINGDIR}/pressio \
	 -all-tpls-path=${PYWORKINGDIR}/tpls \
	 -wipe-existing=1 \
	 -build-mode=Release \
	 -target-type=dynamic \
	 -with-cmake-fnc=sisc_paper_burgpython \
	 -with-packages=rom
    cd ${PYWORKINGDIR}
else
    cd ${PYWORKINGDIR}/tpls/pressio/build
    make install
    cd ${PYWORKINGDIR}
fi

# set paths for eigen, pybind11 and pressio
EIGENPATH="${PYWORKINGDIR}/tpls/eigen/install/include/eigen3"
PYBIND11PATH="${PYWORKINGDIR}/tpls/pybind11/install"
PRESSIOPATH="${PYWORKINGDIR}/tpls/pressio/install/include"

# build pressio4py
bdirname=build
#check if build dir exists
[[ ! -d ${bdirname} ]] && mkdir ${bdirname}
# enter
cd ${bdirname}
cmake -DCMAKE_C_COMPILER=${CC} \
      -DCMAKE_CXX_COMPILER=${CXX} \
      -DCMAKE_VERBOSE_MAKEFILE:BOOL=TRUE \
      -DCMAKE_BUILD_TYPE=Release \
      -DEIGEN_INCLUDE_DIR=${EIGENPATH} \
      -DPYBIND11_DIR=${PYBIND11PATH} \
      -DPRESSIO_INCLUDE_DIR=${PRESSIOPATH} \
      ../pressio4py
make -j6

# I also need the ops
cp ../pressio4py/src/ops/pressio4pyOps.py .
cd ..

# go back where we started
cd ${topDir}
