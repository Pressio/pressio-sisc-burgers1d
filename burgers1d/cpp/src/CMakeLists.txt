
cmake_minimum_required(VERSION 2.8.12)
project(burgers1dcpp)

include_directories(${EIGEN_INCLUDE_DIR} ${PRESSIO_INCLUDE_DIR})

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.hpp)

set(SOURCES_FOM
  ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.cc
  main_fom.cc
)
set(SOURCES_ROM
  ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.cc
  main_rom.cc
)

# fom executable
add_executable(burgers1d_fom ${SOURCES_FOM} ${HEADERS})

# rom executable
add_executable(burgers1d_rom ${SOURCES_ROM} ${HEADERS})
