#!/bin/bash

# top dir where this lives
TOPDIR=${PWD}

# which branch to use for pressio
pressioBranch=develop
# which branch to use for pressio-builder
pressioBuilderBranch=master
# which branch to use for pressio4py
pressioFourPyBranch=master

# burgers1d python sources
PYSRC=${TOPDIR}/python/src

# store the working dir
WORKINGDIR=

# store the working dir for cpp,
# which is WORKINGDIR/python
PYWORKINGDIR=

# yes/no wipe existing data content of target directory
WIPEEXISTING=no

# if we want to solver burgers1d with dense vs sparse data (yes/no)
JACOBIANTYPE=empty

# name of the task to run
WHICHTASK=

# env script
SETENVscript=

# var to detect the os type [linux or mac]
ARCH=
if [[ $OSTYPE == *"darwin"* ]]; then
    ARCH=mac
else
    ARCH=linux
fi

# if we want debug prints on
WITHDBGPRINT=no



function wipe_existing_data_in_target_dir(){
    echo "Wiping $1/{data_*, build}"
    rm -rf $1/build $1/data_*
}


function print_global_vars(){
    echo "TOPDIR			= $TOPDIR"
    echo "PYSRC			= $PYSRC"
    echo "WORKINGDIR		= $WORKINGDIR"
    echo "PYWORKINGDIR		= $PYWORKINGDIR"
    echo "WIPEEXISTING		= ${WIPEEXISTING}"
    echo "SETENVscript		= $SETENVscript"
    echo "TASKNAME		= $TASKNAME"
    echo "JACOBIANTYPE		= $JACOBIANTYPE"
    echo "ARCH			= $ARCH"
    echo "WITHDBGPRINT		= $WITHDBGPRINT"
    echo "Pressio branch		= $pressioBranch"
    echo "Pressio-build branch	= $pressioBuilderBranch"
    echo "Pressio4py branch	= $pressioFourPyBranch"
}

function check_minimum_vars_set(){
    if [ -z $WORKINGDIR ]; then
	echo "--working-dir is empty, must be set: exiting"
	exit 11
    fi
}

function check_jacobian_var(){
    if [ $JACOBIANTYPE = empty ]; then
	echo "--jac-type is empty, must be set to dense or sparse"
	exit 11
    fi
    if [ ${JACOBIANTYPE} != dense ] &&\
	   [ ${JACOBIANTYPE} != sparse ];
    then
	echo "--jac-type is set to non-admissible value"
	echo "choose one of: dense, sparse"
	exit 0
    fi
}

function check_minimum_vars_set_python(){
    # check for common
    check_minimum_vars_set

    # the jacobian type matters always except for rk4 or galerkin
    if [ $WHICHTASK = "lspg" ]; then
	check_jacobian_var
    fi

    if [ ${WHICHTASK} != build ] &&\
	   [ ${WHICHTASK} != lspg ] &&\
	   [ ${WHICHTASK} != galerkin ];
    then
	echo "--do is set to non-admissible value"
	echo "choose one of: build, lspg, galerkin"
	exit 0
    fi
}
