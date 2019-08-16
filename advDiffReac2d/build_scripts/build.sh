#!/bin/bash

# -----------------------------------------------
# purpose: builds all exe for the C++ AdvReadDiff2d
# this script is run by do_all_cpp.sh at the top level directory
# -----------------------------------------------

# go to working dir
cd ${CPPWORKINGDIR}

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

#-------------
# get tpls
#-------------
# first do eigen
if [ ! -d ${CPPWORKINGDIR}/tpls/eigen ]; then
    cd pressio_auto_build/tpls
    bash main_tpls.sh -arch=mac \
	 --with-libraries=eigen \
	 -with-cmake-line-fncs=default \
	 --target-dir=${CPPWORKINGDIR}/tpls \
	 --wipe-existing=1 \
	 -target-type=dynamic
    cd ${CPPWORKINGDIR}
fi
# do trilinos
if [ ! -d ${CPPWORKINGDIR}/tpls/trilinos ]; then
    cd pressio_auto_build/tpls

    if [[ $ONMAC == 1 ]]; then
	bash main_tpls.sh -arch=mac \
	     --with-libraries=trilinos \
	     -with-cmake-line-fncs=sisc_paper_adrcpp_mac \
	     --target-dir=${CPPWORKINGDIR}/tpls \
	     --wipe-existing=1 \
	     -target-type=dynamic
    else
	echo "fill in cmake line for trilinos for non mac"
	exit 1
    fi

    cd ${CPPWORKINGDIR}
fi


# install pressio
# (no tests, just install with cmake which we need because of the cmakedefines)
# Note that:
# arch: does not matter for now
# target-type: does not matter since nothing is compiled yet
if [ ! -d ${CPPWORKINGDIR}/tpls/pressio ]; then
    cd pressio_auto_build/pressio
    bash main_pressio.sh -arch=mac \
	 -target-dir=${CPPWORKINGDIR}/tpls \
	 -pressio-src=${CPPWORKINGDIR}/pressio \
	 -all-tpls-path=${CPPWORKINGDIR}/tpls \
	 -wipe-existing=1 \
	 -build-mode=Release \
	 -target-type=dynamic \
	 -with-cmake-fnc=sisc_paper_adr2dcpp \
	 -with-packages=rom
    cd ${CPPWORKINGDIR}
else
    cd ${CPPWORKINGDIR}/tpls/pressio/build
    make install
    cd ${CPPWORKINGDIR}
fi

# set paths for eigen, trilinos and pressio
EIGENPATH="${CPPWORKINGDIR}/tpls/eigen/install/include/eigen3"
TRILINOSINCPATH="${CPPWORKINGDIR}/tpls/trilinos/install/include"
TRILINOSLIBPATH="${CPPWORKINGDIR}/tpls/trilinos/install/lib"
PRESSIOPATH="${CPPWORKINGDIR}/tpls/pressio/install/include"

# build C++ exes
bdirname=build
#check if build dir exists
if [ ! -d ${bdirname} ]; then
    mkdir ${bdirname}
else
    # enter
    cd ${bdirname} && rm -rf *
    cmake -DCMAKE_C_COMPILER=${CC} \
	  -DCMAKE_CXX_COMPILER=${CXX} \
	  -DCMAKE_VERBOSE_MAKEFILE:BOOL=TRUE \
	  -DCMAKE_BUILD_TYPE=Release \
	  -DEIGEN_INCLUDE_DIR=${EIGENPATH} \
	  -DTRILINOS_INCLUDE_DIR=${TRILINOSINCPATH} \
	  -DTRILINOS_LIBRARY_DIR=${TRILINOSLIBPATH} \
	  -DPRESSIO_INCLUDE_DIR=${PRESSIOPATH} \
	  ${CPPSRC}
    make -j6
    cd ..
fi

# go back where we started
cd ${topDir}
