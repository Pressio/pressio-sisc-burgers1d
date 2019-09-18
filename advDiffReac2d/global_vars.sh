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

# bool to wipe existing content of target directory
WIPEEXISTING=0

# env script
SETENVscript=

WHICHTASK=

# var to detect if we are on mac
if [[ $OSTYPE == *"darwin"* ]]; then
    ONMAC=1
fi

function print_global_vars(){
    echo "TOPDIR	 = $TOPDIR"
    echo "CPPSRC	 = $CPPSRC"
    echo "WORKINGDIR     = $WORKINGDIR"
    echo "CPPWORKINGDIR  = $CPPWORKINGDIR"
    echo "WIPEEXISTING   = ${WIPEEXISTING}"
    echo "SETENVscript   = $SETENVscript"
    echo "WHICHTASK	 = $WHICHTASK"
    echo "ONMAC		 = $ONMAC"
}

function display_admissible_options_for_do(){
    echo " "
    echo " To build all executables: --do=build"
    echo " "
    echo " Eigen FOM choices:"
    echo " - eigen_ms_rk4, eigen_ms_bdf1"
    echo " - eigen_chem_fom_rk4_timing", "eigen_chem_fom_bdf1_timing"
    echo " - eigen_chem_fom_rk4_basis",  "eigen_chem_fom_bdf1_basis"
    echo " "
    echo " Eigen ROM choices:"
    echo " - eigen_chem_lspg_full_mesh_bdf1_timing"
    echo " - eigen_chem_lspg_sample_mesh_bdf1_timing"
    echo " "
    echo " Kokkos FOM choices:"
    echo " - kokkos_ms_rk4"
    echo " - kokkos_chem_fom_rk4_timing"
    echo " "
    echo " Kokkos ROM choices:"
    echo " - kokkos_chem_lspg_full_mesh_bdf1_timing"
    echo " - kokkos_chem_lspg_sample_mesh_bdf1_timing"
    echo " "
}

function check_minimum_vars_set(){
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

    if [[ ${WHICHTASK} != build &&
	      ${WHICHTASK} != eigen_ms_rk4 &&
	      ${WHICHTASK} != eigen_ms_bdf1 &&
	      #
	      ${WHICHTASK} != eigen_chem_fom_rk4_timing	&&
	      ${WHICHTASK} != eigen_chem_fom_bdf1_timing &&
	      ${WHICHTASK} != eigen_chem_fom_rk4_basis &&
	      ${WHICHTASK} != eigen_chem_fom_bdf1_basis &&
	      #
	      ${WHICHTASK} != eigen_chem_lspg_full_mesh_bdf1_timing &&
	      ${WHICHTASK} != eigen_chem_lspg_sample_mesh_bdf1_timing &&
	      #
	      ${WHICHTASK} != kokkos_ms_rk4 &&
	      ${WHICHTASK} != kokkos_chem_fom_rk4_timing &&
	      #
	      ${WHICHTASK} != kokkos_chem_lspg_full_mesh_bdf1_timing &&
	      ${WHICHTASK} != kokkos_chem_lspg_sample_mesh_bdf1_timing ]];
    then
	echo "--do is set to non-admissible value, choose one of:"
	display_admissible_options_for_do
	exit 3
    fi
}
