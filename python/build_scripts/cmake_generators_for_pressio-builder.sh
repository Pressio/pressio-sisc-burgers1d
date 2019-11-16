#!/bin/bash

function pressio_sisc_burgerspython(){
    pressio_build_type
    pressio_serial_c_cxx_compilers
    pressio_enable_eigen
    pressio_enable_pybind11
}

function pressio_sisc_burgerspython_dbgprint(){
    pressio_sisc_burgerspython
    pressio_enable_debug_print
}
