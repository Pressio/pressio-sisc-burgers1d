cmake_minimum_required(VERSION 3.11.0)
project(burgers1dcpp)
set (CMAKE_CXX_STANDARD 11)

include_directories(${EIGEN_INCLUDE_DIR} ${PRESSIO_INCLUDE_DIR})
link_directories(${BLAS_LIB_DIR})

#add_definitions(-DEIGEN_USE_BLAS)

set(HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.hpp
)
set(SOURCES_FOM_RK4  ${CMAKE_CURRENT_SOURCE_DIR}/main_fom_rk4.cc)
set(SOURCES_ROM_GAL  ${CMAKE_CURRENT_SOURCE_DIR}/main_rom_galerkin.cc)
set(SOURCES_FOM_BDF1 ${CMAKE_CURRENT_SOURCE_DIR}/main_fom_bdf1.cc)
set(SOURCES_ROM_LSPG ${CMAKE_CURRENT_SOURCE_DIR}/main_rom_lspg.cc)

# fom with rk4
add_executable(burgers1d_fom_rk4 ${SOURCES_FOM_RK4} ${HEADERS})
target_link_libraries(burgers1d_fom_rk4 openblas)

# galerkin (rk4)
add_executable(burgers1d_rom_galerkin ${SOURCES_ROM_GAL} ${HEADERS})
target_link_libraries(burgers1d_rom_galerkin openblas)

# bdf1
add_executable(burgers1d_fom_bdf1 ${SOURCES_FOM_BDF1} ${HEADERS})
target_link_libraries(burgers1d_fom_bdf1 openblas)

# lspg
add_executable(burgers1d_rom_lspg ${SOURCES_ROM_LSPG} ${HEADERS})
target_link_libraries(burgers1d_rom_lspg openblas)











# if(${HAVE_DENSE})
#   message("You enabled Dense matrices")
#   add_definitions(-DUSE_DENSE)
# endif()

#set(EIGEN_USE_BLAS ON)
#set(EIGEN_USE_LAPACKE ON)
#link_directories(${BLAS_LIB_DIR} ${LAPACK_LIB_DIR})
#set(SOURCES_FOM_RK4  ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.cc main_fom_rk4.cc)
#set(SOURCES_ROM_GALERKIN  ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.cc main_rom_galerkin.cc)
