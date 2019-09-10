#!/bin/bash

function pressio_mac_sisc_paper_burgpython(){
    pressio_always_needed
    pressio_serial_c_cxx_compilers
    pressio_fortran_off
    pressio_tests_off
    pressio_examples_off
    pressio_enable_eigen
    pressio_enable_pybind11
    pressio_pressio_packages
}
