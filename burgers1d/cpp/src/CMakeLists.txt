
cmake_minimum_required(VERSION 2.8.12)
project(burgers1dcpp)

include_directories(${EIGEN_INCLUDE_DIR} ${PRESSIO_INCLUDE_DIR})

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.hpp)

set(SOURCES_FOM_BDF1 ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.cc main_fom_bdf1.cc)
set(SOURCES_ROM_LSPG ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.cc main_rom_lspg.cc)
set(SOURCES_FOM_RK4  ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.cc main_fom_rk4.cc)
set(SOURCES_ROM_GALERKIN  ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.cc main_rom_galerkin.cc)

add_executable(burgers1d_fom_bdf1 ${SOURCES_FOM_BDF1} ${HEADERS})
add_executable(burgers1d_fom_rk4 ${SOURCES_FOM_RK4} ${HEADERS})
add_executable(burgers1d_rom_lspg ${SOURCES_ROM_LSPG} ${HEADERS})
add_executable(burgers1d_rom_galerkin ${SOURCES_ROM_GALERKIN} ${HEADERS})
