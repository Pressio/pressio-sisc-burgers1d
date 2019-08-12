#!/bin/bash

# top dir where this lives
topDir=${PWD}

# burgers1d cpp source
CPPSRC=${topDir}/cpp/src

# store the working dir
WORKINGDIR=

# store the working dir for cpp,
# which is WORKINGDIR/cpp
CPPWORKINGDIR=

# bool to wipe existing content of target directory
WIPEEXISTING=0

WHICHTASK=

# env script
SETENVscript=

# var to detect if we are on mac
if [[ $OSTYPE == *"darwin"* ]]; then
    ONMAC=1
fi

print_global_vars(){
    echo "TOPDIR	 = $TOPDIR"
    echo "CPPSRC	 = $CPPSRC"
    echo "WORKINGDIR     = $WORKINGDIR"
    echo "WIPEEXISTING   = ${WIPEEXISTING}"
    echo "SETENVscript   = $SETENVscript"
}

check_minimum_vars_set(){
    if [[ -z $WORKINGDIR ]]; then
	echo "--working-dir is empty, must be set: exiting"
	exit 1
    fi
    if [[ -z $WHICHTASK ]]; then
	echo "--do is empty, must be set"
	echo " for do_all_cpp, choose one of: build"
	exit 2
    fi
}

check_minimum_vars_set_cpp(){
    # check for common
    check_minimum_vars_set

    if [[ ${WHICHTASK} != build ]]; then
	echo "--do is set to non-admissible value"
	echo "choose one of: build"
	exit 3
    fi
}
