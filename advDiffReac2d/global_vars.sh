#!/bin/bash

# top dir where this lives or is sourced
topDir=${PWD}

# source
CPPSRC=${topDir}/src

# the working dir
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

display_admissible_options_for_do(){
    echo " * build"
    echo " * eigen_ms_rk4, eigen_ms_bdf1"
    echo " * eigen_chem_fom_rk4_timing", "eigen_chem_fom_bdf1_timing"
    echo " * eigen_chem_fom_rk4_basis", "eigen_chem_fom_bdf1_basis"
}

check_minimum_vars_set(){
    if [[ -z $WORKINGDIR ]]; then
	echo "--working-dir is empty, must be set: exiting"
	exit 1
    fi
    if [[ -z $WHICHTASK ]]; then
	echo "--do is empty, must be set"
	echo " for do_all, choose one of: "
	display_admissible_options_for_do
	exit 2
    fi
}

check_minimum_vars_set_cpp(){
    # check for common
    check_minimum_vars_set

    if [[ ${WHICHTASK} != build &&
	      ${WHICHTASK} != eigen_ms_rk4 &&
	      ${WHICHTASK} != eigen_ms_bdf1 &&
	      ${WHICHTASK} != eigen_chem_fom_rk4_timing &&
	      ${WHICHTASK} != eigen_chem_fom_bdf1_timing &&
	      ${WHICHTASK} != eigen_chem_fom_rk4_basis &&
	      ${WHICHTASK} != eigen_chem_fom_bdf1_basis ]];
    then
	echo "you set --do to a non-admissible value, choose one of:"
	display_admissible_options_for_do
	exit 3
    fi
}
