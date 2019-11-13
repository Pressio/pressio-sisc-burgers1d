
cmake_minimum_required(VERSION 3.11.0)
project(burgers1dcpp)
set (CMAKE_CXX_STANDARD 11)

include_directories(${EIGEN_INCLUDE_DIR} ${PRESSIO_INCLUDE_DIR})
if(${HAVE_BLASLAPACK})
  link_directories(${BLAS_LIBDIR} ${LAPACK_LIBDIR})
  add_definitions(-DEIGEN_USE_BLAS -DEIGEN_USE_LAPACKE)
endif()

if(${HAVE_DENSE})
  message("Dense matrices enabled")
  add_definitions(-DUSE_DENSE)
endif()

set(HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.hpp
)
set(SOURCES_FOM_RK4  ${CMAKE_CURRENT_SOURCE_DIR}/main_fom_rk4.cc)
set(SOURCES_ROM_GAL  ${CMAKE_CURRENT_SOURCE_DIR}/main_rom_galerkin.cc)
set(SOURCES_FOM_BDF1 ${CMAKE_CURRENT_SOURCE_DIR}/main_fom_bdf1.cc)
set(SOURCES_ROM_LSPG ${CMAKE_CURRENT_SOURCE_DIR}/main_rom_lspg.cc)

# fom with rk4
add_executable(burgers1d_fom_rk4 ${SOURCES_FOM_RK4} ${HEADERS})
if(${HAVE_BLASLAPACK})
  target_link_libraries(burgers1d_fom_rk4 ${BLAS_NAME} ${LAPACK_NAME})
endif()

# galerkin (rk4)
add_executable(burgers1d_rom_galerkin ${SOURCES_ROM_GAL} ${HEADERS})
if(${HAVE_BLASLAPACK})
  target_link_libraries(burgers1d_rom_galerkin ${BLAS_NAME} ${LAPACK_NAME})
endif()

# bdf1
add_executable(burgers1d_fom_bdf1 ${SOURCES_FOM_BDF1} ${HEADERS})
if(${HAVE_BLASLAPACK})
  target_link_libraries(burgers1d_fom_bdf1 ${BLAS_NAME} ${LAPACK_NAME})
endif()

# lspg
add_executable(burgers1d_rom_lspg ${SOURCES_ROM_LSPG} ${HEADERS})
if(${HAVE_BLASLAPACK})
  target_link_libraries(burgers1d_rom_lspg ${BLAS_NAME} ${LAPACK_NAME})
endif()
