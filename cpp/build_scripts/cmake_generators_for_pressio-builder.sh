#!/bin/bash

function tril_sisc_burgerscpp(){
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

function pressio_sisc_burgerscpp(){
    pressio_build_type
    pressio_cmake_verbose
    pressio_serial_c_cxx_compilers
    pressio_serial_fortran_compiler
    pressio_openblaslapack
    pressio_enable_eigen
}

function pressio_sisc_burgerscpp_dbgprint(){
    pressio_sisc_burgerscpp
    pressio_enable_debug_print
}
