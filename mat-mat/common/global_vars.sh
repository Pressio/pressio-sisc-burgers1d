#!/bin/bash

# top dir where this lives
TOPDIR=${PWD}

# burgers1d cpp source
CPPSRC=${TOPDIR}/cpp/src

# burgers1d python sources
PYSRC=${TOPDIR}/python/src

# store the working dir
WORKINGDIR=

# store the working dir for cpp,
# which is WORKINGDIR/cpp
CPPWORKINGDIR=

# store the working dir for cpp,
# which is WORKINGDIR/python
PYWORKINGDIR=

# yes/no wipe existing data content of target directory
WIPEEXISTING=no

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

# which branch to use for pressio
pressioBranch=siscPaper

# which branch to use for pressio-builder
pressioBuilderBranch=master

# which branch to use for pressio4py
pressioFourPyBranch=master


function wipe_existing_data_in_target_dir(){
    echo "Wiping $1/{data_*, build}"
    rm -rf $1/build $1/data_*
}


function print_global_vars(){
    echo "TOPDIR			= $TOPDIR"
    echo "CPPSRC			= $CPPSRC"
    echo "PYSRC			= $PYSRC"
    echo "WORKINGDIR		= $WORKINGDIR"
    echo "CPPWORKINGDIR		= $CPPWORKINGDIR"
    echo "PYWORKINGDIR		= $PYWORKINGDIR"
    echo "WIPEEXISTING		= ${WIPEEXISTING}"
    echo "SETENVscript		= $SETENVscript"
    echo "TASKNAME		= $TASKNAME"
    echo "ARCH			= $ARCH"
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

function check_minimum_vars_set_cpp(){
    # check for common
    check_minimum_vars_set

    if [ ${WHICHTASK} != build ] &&\
	   [ ${WHICHTASK} != dgemm ] &&\
	   [ ${WHICHTASK} != spgemm ];
    then
	echo "--do is set to non-admissible value"
	echo "choose one of: build, dgemm, spgemm"
	exit 0
    fi
}

function check_minimum_vars_set_python(){
    # check for common
    check_minimum_vars_set

    if [ ${WHICHTASK} != build ] &&\
	   [ ${WHICHTASK} != dgemm ] &&\
	   [ ${WHICHTASK} != spgemm ];
    then
	echo "--do is set to non-admissible value"
	echo "choose one of: build, dgemm, spgemm"
	exit 0
    fi
}
