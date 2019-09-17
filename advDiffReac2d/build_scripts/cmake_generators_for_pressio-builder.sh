#!/bin/bash

#---------------------------------
# trilinos generators
#---------------------------------
function tril_mac_sisc_paper_adr2dcpp(){
    trilinos_build_type
    trilinos_link_type
    trilinos_verbose_makefile_on
    trilinos_mpi_c_cxx_compilers
    trilinos_mpi_fortran_on
    trilinos_tests_off
    trilinos_examples_off
    trilinos_kokkos_serial
    trilinos_openblaslapack
    trilinos_packages_for_pressio
}

#---------------------------------
# pressio generators
#---------------------------------
function pressio_mac_sisc_paper_adr2dcpp(){
    pressio_build_type
    pressio_link_type
    pressio_mpi_c_cxx_compilers
    pressio_mpi_fortran_on
    pressio_tests_on
    pressio_examples_off
    pressio_openblaslapack
    pressio_enable_eigen
    pressio_enable_trilinos
    pressio_pressio_target_package
    pressio_enable_debug_print
}
