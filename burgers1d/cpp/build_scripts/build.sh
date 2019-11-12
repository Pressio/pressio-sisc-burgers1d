#!/bin/bash

# -----------------------------------------------
# purpose: builds all exe for the C++ Burgers1d
# this script is run by do_all_cpp.sh at the top level directory
# -----------------------------------------------

set -e

#-------------------
# go to working dir
#-------------------
cd ${CPPWORKINGDIR}

# clone pressio-builder
if [ ! -d ${CPPWORKINGDIR}/pressio-builder ]; then
    git clone git@github.com:Pressio/pressio-builder.git
    cd pressio-builder
    git checkout ${pressioBuilderBranch}
    cd ..
else
    cd pressio-builder && git pull && cd -
fi

#-------------
# do eigen
#-------------
if [ ! -d ${CPPWORKINGDIR}/tpls/eigen ]; then
    cd ${CPPWORKINGDIR}/pressio-builder
    ./main_tpls.sh \
	-dryrun=no --tpls=eigen -build-mode=Release \
	-target-dir=${CPPWORKINGDIR}/tpls --wipe-existing=1
    cd ${CPPWORKINGDIR}
fi

#-------------
# do pressio
#-------------
# (install with cmake which we need because of the cmakedefines)
if [[ ! -d ${CPPWORKINGDIR}/tpls/pressio ||\
	 ! -d ${CPPWORKINGDIR}/tpls/pressio/build ]];
then
    [ ! -d ${CPPWORKINGDIR}/tpls/pressio ] && mkdir -p ${CPPWORKINGDIR}/tpls/pressio
    cd ${CPPWORKINGDIR}/tpls/pressio

    # clone the repo
    if [ ! -d ${CPPWORKINGDIR}/tpls/pressio/pressio ]; then
	git clone --recursive git@github.com:Pressio/pressio.git
    fi
    cd pressio
    git checkout ${pressioBranch}
    cd ..

    # the generator line
    PRESSIOGENFNCNAME=pressio_sisc_burgerscpp
    if [[ $WITHDBGPRINT == yes ]]; then
	PRESSIOGENFNCNAME=pressio_sisc_burgerscpp_dbgprint
    fi

    # install pressio
    cd ${CPPWORKINGDIR}/pressio-builder
    ./main_pressio.sh \
	-dryrun=no \
	-pressio-src=${CPPWORKINGDIR}/tpls/pressio/pressio \
	-target-dir=${CPPWORKINGDIR}/tpls \
	-wipe-existing=yes \
	-build-mode=Release \
	-cmake-custom-generator-file=${TOPDIR}/cpp/build_scripts/cmake_generators_for_pressio-builder.sh \
	-cmake-generator-name=${PRESSIOGENFNCNAME} \
	-eigen-path=${CPPWORKINGDIR}/tpls/eigen/install

    cd ${CPPWORKINGDIR}
else
    #cd ${CPPWORKINGDIR}/tpls/pressio/pressio && git pull && cd -
    cd ${CPPWORKINGDIR}/tpls/pressio/build
    make -j4 install
    cd ${CPPWORKINGDIR}
fi

# set paths for eigen and pressio
EIGENPATH="${CPPWORKINGDIR}/tpls/eigen/install/include/eigen3"
PRESSIOPATH="${CPPWORKINGDIR}/tpls/pressio/install/include"

USEDENSE=OFF
[[ ${JACOBIANTYPE} == dense ]] && USEDENSE=ON

USEBLAS=ON
[[ ${WITHNATIVEEIGEN} == yes ]] && USEBLAS=OFF

# build Burgers1d C++ exes
bdirname=build
#check if build dir exists
if [ ! -d ${bdirname} ]; then
    mkdir ${bdirname}
fi
# enter
cd ${bdirname} && rm -rf *
cmake -DCMAKE_C_COMPILER=${CC} \
      -DCMAKE_CXX_COMPILER=${CXX} \
      -DCMAKE_VERBOSE_MAKEFILE:BOOL=TRUE \
      -DCMAKE_BUILD_TYPE=Release \
      -DEIGEN_INCLUDE_DIR=${EIGENPATH} \
      -DPRESSIO_INCLUDE_DIR=${PRESSIOPATH} \
      -DHAVE_DENSE:BOOL=${USEDENSE} \
      -DBLAS_LIB_DIR=${BLAS_ROOT}/lib \
      -DLAPACK_LIB_DIR=${LAPACK_ROOT}/lib \
      -DHAVE_BLASLAPACK:BOOL=${USEBLAS}\
      -DCMAKE_CXX_FLAGS="-march=native"\
      ${CPPSRC}
make -j6

# go back where we started
cd ${TOPDIR}
