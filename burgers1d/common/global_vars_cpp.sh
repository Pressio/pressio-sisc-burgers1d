#!/bin/bash

# top dir where this lives
TOPDIR=${PWD}

# burgers1d cpp source
CPPSRC=${TOPDIR}/cpp/src

# store the working dir
WORKINGDIR=

# store the working dir for cpp,
# which is WORKINGDIR/cpp
CPPWORKINGDIR=

# yes/no wipe existing data content of target directory
WIPEEXISTING=no

# location of trilinos
TRILINOSPFX=

# if we want to solver burgers1d with dense vs sparse data (yes/no)
DENSEJACOBIAN=no

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

# which branch to use for pressio
pressioBranch=siscPaper

# which branch to use for pressio-builder
pressioBuilderBranch=master


function wipe_existing_data_in_target_dir(){
    echo "Wiping $1/{data_*, build}"
    rm -rf $1/build $1/data_*
}


function print_global_vars(){
    echo "TOPDIR			= $TOPDIR"
    echo "CPPSRC			= $CPPSRC"
    echo "WORKINGDIR		= $WORKINGDIR"
    echo "CPPWORKINGDIR		= $CPPWORKINGDIR"
    echo "WIPEEXISTING		= ${WIPEEXISTING}"
    echo "SETENVscript		= $SETENVscript"
    echo "TASKNAME		= $TASKNAME"
    echo "DENSEJACOBIAN		= $DENSEJACOBIAN"
    echo "ARCH			= $ARCH"
    echo "WITHDBGPRINT		= $WITHDBGPRINT"
    echo "Pressio branch		= $pressioBranch"
    echo "Pressio-build branch	= $pressioBuilderBranch"
}

function check_minimum_vars_set(){
    if [ -z $WORKINGDIR ]; then
	echo "--working-dir is empty, must be set: exiting"
	exit 11
    fi
}

function check_minimum_vars_set_plot(){
    # check for common
    check_minimum_vars_set

    if [ ${WHICHTASK} != lspg ] &&\
	   [ ${WHICHTASK} != galerkin ];
    then
	echo "--do is set to non-admissible value"
	echo "choose one of: lspg, galerkin"
	exit 0
    fi
}

function check_minimum_vars_set_cpp(){
    # check for common
    check_minimum_vars_set

    if [ ${WHICHTASK} != build ] &&\
	   [ ${WHICHTASK} != fom_bdf1_timing ] &&\
	   [ ${WHICHTASK} != fom_bdf1_basis ] &&\
	   [ ${WHICHTASK} != lspg ] &&\
	   [ ${WHICHTASK} != fom_rk4_timing ] &&\
	   [ ${WHICHTASK} != fom_rk4_basis ] &&\
	   [ ${WHICHTASK} != galerkin ];
    then
	echo "--do is set to non-admissible value"
	echo "choose one of: build, fom_bdf1_timing,"\
	     "fom_bdf1_basis, lspg "\
	     "fom_rk4_timing, fom_rk4_basis, galerkin"
	exit 0
    fi
}
