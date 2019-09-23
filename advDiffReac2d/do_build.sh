#!/bin/bash

set -e

# -----------------------------------------------
# build the executables
# -----------------------------------------------

# load global variables
source ${PWD}/global_vars.sh

# parse cline arguments
source ${PWD}/cmd_line_options.sh

# check that all basic variables are set, otherwise leave
check_minimum_vars_set

echo ""
echo "--------------------------------------------"
echo " current setting is: "
echo ""
print_global_vars
echo ""
echo "--------------------------------------------"

# set env if not already set
if [[ ! -z ${SETENVscript} ]]; then
    echo "loading environment from ${SETENVscript}"
    source ${SETENVscript}
    echo "PATH = $PATH"
else
    echo "--with-env-script NOT set, so we assume env is set already"
fi

# create working dir if not existing
[[ ! -d ${WORKINGDIR} ]] && mkdir ${WORKINGDIR}

# create subworking dir if not existing
CPPWORKINGDIR=${WORKINGDIR}/cpp
[[ ! -d ${CPPWORKINGDIR} ]] && mkdir ${CPPWORKINGDIR}

# wipe everything if set to 1
[[ $WIPEEXISTING == yes ]] && wipe_existing_data_in_target_dir


#---------------------------
# go to working dir
cd ${CPPWORKINGDIR}

# clone pressio-builder
if [ ! -d ${CPPWORKINGDIR}/pressio-builder ];
then
    git clone git@github.com:Pressio/pressio-builder.git
    #cd pressio-builder && git checkout siscPaper && cd ..
else
    cd pressio-builder && git pull && cd -
fi

#-------------
# do eigen
#-------------
if [ ! -d ${CPPWORKINGDIR}/tpls/eigen ]; then
    cd ${CPPWORKINGDIR}/pressio-builder
    ./main_tpls.sh \
	--dryrun=no \
	--tpls=eigen \
	--build-mode=Release\
	--target-dir=${CPPWORKINGDIR}/tpls \
	--wipe-existing=1
    cd ${CPPWORKINGDIR}
fi

#-------------
# do gtest
#-------------
if [ ! -d ${CPPWORKINGDIR}/tpls/gtest ]; then
    cd ${CPPWORKINGDIR}/pressio-builder
    ./main_tpls.sh \
	--dryrun=no \
	--tpls=gtest \
	--target-dir=${CPPWORKINGDIR}/tpls \
	--wipe-existing=1
    cd ${CPPWORKINGDIR}
fi

# do trilinos
if [ ! -d ${CPPWORKINGDIR}/tpls/trilinos ]; then
    cd ${CPPWORKINGDIR}/pressio-builder

    if [[ $ONMAC -eq 1 ]]; then
	./main_tpls.sh \
	    -dryrun=no \
	    -tpls=trilinos \
	    -target-dir=${CPPWORKINGDIR}/tpls \
	    -build-mode=Release\
	    -wipe-existing=yes \
	    -link-type=dynamic \
	    -cmake-custom-generator-file=${TOPDIR}/cmake_generators_for_pressio-builder.sh \
	    -cmake-generator-names=tril_mac_sisc_paper_adr2dcpp
    else
	./main_tpls.sh \
	    -dryrun=no \
	    -tpls=trilinos \
	    -target-dir=${CPPWORKINGDIR}/tpls \
	    -build-mode=Release\
	    -wipe-existing=yes \
	    -link-type=dynamic \
	    -cmake-custom-generator-file=${TOPDIR}/cmake_generators_for_pressio-builder.sh \
	    -cmake-generator-names=tril_linux_sisc_paper_adr2dcpp
    fi

    cd ${CPPWORKINGDIR}
fi

#-------------
# do pressio
#-------------
# (install with cmake which we need because of the cmakedefines)
# only need rom package (others turned on automatically)
# target-type: does not matter since Pressio is NOT compiled yet
if [[ ! -d ${CPPWORKINGDIR}/tpls/pressio ||\
	 ! -d ${CPPWORKINGDIR}/tpls/pressio/build ]];
then
    [ ! -d ${CPPWORKINGDIR}/tpls/pressio ] && mkdir -p ${CPPWORKINGDIR}/tpls/pressio
    cd ${CPPWORKINGDIR}/tpls/pressio

    # clone the repo
    if [ ! -d ${CPPWORKINGDIR}/tpls/pressio/pressio ]; then
	git clone --recursive git@github.com:Pressio/pressio.git
    fi
    cd pressio && git checkout develop && cd ..

    # install pressio
    cd ${CPPWORKINGDIR}/pressio-builder
    if [[ $ONMAC -eq 1 ]];
    then
	./main_pressio.sh \
	    -dryrun=no \
	    -pressio-src=${CPPWORKINGDIR}/tpls/pressio/pressio \
	    -target-dir=${CPPWORKINGDIR}/tpls \
	    -package-name=rom \
	    -wipe-existing=yes \
	    -build-mode=Release \
	    -link-type=dynamic \
	    -cmake-custom-generator-file=${TOPDIR}/cmake_generators_for_pressio-builder.sh \
	    -cmake-generator-name=pressio_mac_sisc_paper_adr2dcpp \
	    -eigen-path=${CPPWORKINGDIR}/tpls/eigen/install \
	    -gtest-path=${CPPWORKINGDIR}/tpls/gtest/install \
	    -trilinos-path=${CPPWORKINGDIR}/tpls/trilinos/install
    else
	echo "fill in cmake line for pressio for linux"
	exit 1
    fi

    cd ${CPPWORKINGDIR}
else
    cd ${CPPWORKINGDIR}/tpls/pressio/pressio && git pull && cd -
    cd ${CPPWORKINGDIR}/tpls/pressio/build
    make -j2 install
    cd ${CPPWORKINGDIR}
fi

#----------------------
# build advDiffReac2d
#----------------------
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
fi
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
      -DBLAS_LIB_DIR=${BLAS_ROOT}/lib \
      -DLAPACK_LIB_DIR=${LAPACK_ROOT}/lib \
      ${CPPSRC}
make -j6
cd ..

# go back where we started
cd ${TOPDIR}
