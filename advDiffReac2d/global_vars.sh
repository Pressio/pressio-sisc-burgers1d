#!/bin/bash

# top dir where this lives or is sourced
TOPDIR=${PWD}

# source
CPPSRC=${TOPDIR}/src

# the working dir
WORKINGDIR=

# store the working dir for cpp,
# which is WORKINGDIR/cpp
CPPWORKINGDIR=

# yes/no wipe existing data content of target directory
WIPEEXISTING=no

# env script
SETENVscript=

# name of the task, depends on what you run
TASKNAME=

# the type of mesh to use to identify the ordering method
# use "natural" for using meshes with natural ordering
# use "rcm" for using meshes ordered with reverse cuthill-mackee
MESHORDERNAME=natural

# var to detect if we are on mac
if [[ $OSTYPE == *"darwin"* ]]; then
    ONMAC=1
fi

function wipe_existing_data_in_target_dir(){
    echo "Wiping existing data in ${CPPWORKINGDIR}"
    rm -rf ${CPPWORKINGDIR}/build
}

function print_global_vars(){
    echo "TOPDIR         = $TOPDIR"
    echo "TASKNAME       = $TASKNAME"
    echo "CPPSRC         = $CPPSRC"
    echo "WORKINGDIR     = $WORKINGDIR"
    echo "CPPWORKINGDIR  = $CPPWORKINGDIR"
    echo "WIPEEXISTING   = ${WIPEEXISTING}"
    echo "SETENVscript   = $SETENVscript"
    echo "ONMAC          = $ONMAC"
}

function check_minimum_vars_set(){
    if [[ -z $WORKINGDIR ]]; then
	echo "--working-dir is empty, must be set: exiting"
	exit 1
    fi
}

#     if [[ -z $TASKNAME ]]; then
# 	echo "--do is empty, must be set"
# 	echo " for do_all, choose one of: "
# 	display_admissible_options_for_do
# 	exit 2
#     fi

#     if [[ ${TASKNAME} != build &&
# 	      ${TASKNAME} != eigen_ms_rk4 &&
# 	      ${TASKNAME} != eigen_ms_bdf1 &&
# 	      #
# 	      ${TASKNAME} != eigen_chem_fom_rk4_timing	&&
# 	      ${TASKNAME} != eigen_chem_fom_bdf1_timing &&
# 	      ${TASKNAME} != eigen_chem_fom_rk4_basis &&
# 	      ${TASKNAME} != eigen_chem_fom_bdf1_basis &&
# 	      #
# 	      ${TASKNAME} != eigen_chem_lspg_full_mesh_bdf1_timing &&
# 	      ${TASKNAME} != eigen_chem_lspg_sample_mesh_bdf1_timing &&
# 	      #
# 	      ${TASKNAME} != kokkos_ms_rk4 &&
# 	      ${TASKNAME} != kokkos_chem_fom_rk4_timing &&
# 	      #
# 	      ${TASKNAME} != kokkos_chem_lspg_full_mesh_bdf1_timing &&
# 	      ${TASKNAME} != kokkos_chem_lspg_sample_mesh_bdf1_timing ]];
#     then
# 	echo "--do is set to non-admissible value, choose one of:"
# 	display_admissible_options_for_do
# 	exit 3
#     fi
# }
