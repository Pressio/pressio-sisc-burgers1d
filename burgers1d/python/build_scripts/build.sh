#!/bin/bash

set -e

# go to working dir
cd ${PYWORKINGDIR}

# clone pressio-builder
if [ ! -d ${PYWORKINGDIR}/pressio-builder ];
then
    git clone git@github.com:Pressio/pressio-builder.git
    cd pressio-builder && git checkout master && cd ..
else
    cd pressio-builder && git pull && cd -
fi

#--------------------
# do eigen
#--------------------
if [ ! -d ${PYWORKINGDIR}/tpls/eigen ]; then
    cd ${PYWORKINGDIR}/pressio-builder
    ./main_tpls.sh \
	--dryrun=no \
	--tpls=eigen \
	--build-mode=Release\
	--target-dir=${PYWORKINGDIR}/tpls \
	--wipe-existing=1
    cd ${PYWORKINGDIR}
fi

#--------------------
# do pybind11
#--------------------
if [ ! -d ${PYWORKINGDIR}/tpls/pybind11 ]; then
    cd ${PYWORKINGDIR}/pressio-builder
    ./main_tpls.sh \
	--dryrun=no \
	--tpls=pybind11 \
	--build-mode=Release\
	--target-dir=${PYWORKINGDIR}/tpls \
	--wipe-existing=1
    cd ${PYWORKINGDIR}
fi


#-------------
# do pressio
#-------------
# (install with cmake which we need because of the cmakedefines)
# only need rom package (others turned on automatically)
# link-type: does not matter since Pressio is NOT compiled yet
if [[ ! -d ${PYWORKINGDIR}/tpls/pressio ||\
	 ! -d ${PYWORKINGDIR}/tpls/pressio/build ]];
then
    [ ! -d ${PYWORKINGDIR}/tpls/pressio ] && mkdir -p ${PYWORKINGDIR}/tpls/pressio
    cd ${PYWORKINGDIR}/tpls/pressio

    # clone the repo
    if [ ! -d ${PYWORKINGDIR}/tpls/pressio/pressio ]; then
	git clone --recursive git@github.com:Pressio/pressio.git
    fi
    cd pressio && git checkout develop && cd ..

    # by default, the generator line is
    PRESSIOGENFNCNAME=pressio_sisc_burgerspython
    if [[ $WITHDBGPRINT == yes ]]; then
	PRESSIOGENFNCNAME=pressio_sisc_burgerspython_dbgprint
    fi

    # install pressio
    cd ${PYWORKINGDIR}/pressio-builder
    ./main_pressio.sh \
	-dryrun=no \
	-pressio-src=${PYWORKINGDIR}/tpls/pressio/pressio \
	-target-dir=${PYWORKINGDIR}/tpls \
	-package-name=rom \
	-wipe-existing=yes \
	-build-mode=Release \
	-link-type=dynamic \
	-cmake-custom-generator-file=${TOPDIR}/python/build_scripts/cmake_generators_for_pressio-builder.sh \
	-cmake-generator-name=${PRESSIOGENFNCNAME} \
	-eigen-path=${PYWORKINGDIR}/tpls/eigen/install \
	-gtest-path=${PYWORKINGDIR}/tpls/gtest/install

    cd ${PYWORKINGDIR}
else
    cd ${PYWORKINGDIR}/tpls/pressio/pressio && git pull && cd -
    cd ${PYWORKINGDIR}/tpls/pressio/build
    make -j4 install
    cd ${PYWORKINGDIR}
fi


# do pressio4py
if [[ ! -d ${PYWORKINGDIR}/tpls/pressio4py ||\
	 ! -d ${PYWORKINGDIR}/tpls/pressio4py/build ]];
then
    if [ ! -d ${PYWORKINGDIR}/tpls/pressio4py ]; then
	mkdir -p ${PYWORKINGDIR}/tpls/pressio4py
    fi

    cd ${PYWORKINGDIR}/tpls/pressio4py
    if [ ! -d pressio4py ]; then
	git clone git@github.com:fnrizzi/pressio4py.git
    fi

    # set paths for eigen, pybind11 and pressio
    EIGENPATH="${PYWORKINGDIR}/tpls/eigen/install/include/eigen3"
    PYBIND11PATH="${PYWORKINGDIR}/tpls/pybind11/install"
    PRESSIOPATH="${PYWORKINGDIR}/tpls/pressio/install/include"

    bdirname=build
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
    cd ..

    cd ${PYWORKINGDIR}
else
    cd ${PYWORKINGDIR}/tpls/pressio4py/pressio4py && git pull && cd -
    cd ${PYWORKINGDIR}/tpls/pressio4py/build
    make -j4
    cd ${PYWORKINGDIR}
fi


# create a build where we put all pybind11 libs and ops
bdirname=build
if [ ! -d ${bdirname} ]; then
    mkdir ${bdirname}
fi
cd ${bdirname} && rm -rf *
ln -s ${PYWORKINGDIR}/tpls/pressio4py/build/pressio4py.*.so pressio4py.so
ln -s ${PYWORKINGDIR}/tpls/pressio4py/build/pressio4pyGalerkin.*.so pressio4pyGalerkin.so
ln -s ${PYWORKINGDIR}/tpls/pressio4py/build/pressio4pyLspg.*.so pressio4pyLspg.so
ln -s ${PYWORKINGDIR}/tpls/pressio4py/pressio4py/src/ops/pressio4pyOps.py pressio4pyOps.py

cd ${TOPDIR}
