#!/bin/bash

function pressio_sisc_burgerscpp(){
    pressio_build_type
    pressio_link_type
    pressio_serial_c_cxx_compilers
    pressio_fortran_off
    pressio_tests_off
    pressio_examples_off
    pressio_enable_eigen
    pressio_pressio_target_package
}

function pressio_sisc_burgerscpp_dbgprint(){
    pressio_sisc_burgerscpp
    pressio_enable_debug_print
}
