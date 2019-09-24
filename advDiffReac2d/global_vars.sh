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

# location of trilinos if passed by user
TRILINOSPFX=

# name of the task, depends on what you run
TASKNAME=

# if we want OpenMP enabled
WITHOPENMP=no

# the type of mesh to use to identify the ordering method
# use "natural" for using meshes with natural ordering
# use "rcm" for using meshes ordered with reverse cuthill-mackee
MESHORDERNAME=natural

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
    echo "Wiping ${CPPWORKINGDIR}/{data_*, build}"
    rm -rf ${CPPWORKINGDIR}/build ${CPPWORKINGDIR}/data_*
}

function print_global_vars(){
    echo "TOPDIR         = $TOPDIR"
    echo "CPPSRC         = $CPPSRC"
    echo "WORKINGDIR     = $WORKINGDIR"
    echo "CPPWORKINGDIR  = $CPPWORKINGDIR"
    echo "WIPEEXISTING   = ${WIPEEXISTING}"
    echo "SETENVscript   = $SETENVscript"
    echo "TRILINOSPFX    = $TRILINOSPFX"
    echo "TASKNAME       = $TASKNAME"
    echo "WITHOPENMP     = $WITHOPENMP"
    echo "MESHORDERNAME  = $MESHORDERNAME"
    echo "ARCH           = $ARCH"
    echo "WITHDBGPRINT   = $WITHDBGPRINT"
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
