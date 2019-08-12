
cmake_minimum_required(VERSION 2.8.12)
project(burgers1dcpp)

include_directories(
  ${EIGEN_INCLUDE_DIR} ${TRILINOS_INCLUDE_DIR} ${PRESSIO_INCLUDE_DIR}
  )
link_directories(${TRILINOS_LIBRARY_DIR})

set(TRILLIBNAMES
  kokkosalgorithms kokkoscontainers kokkoscore teuchoskokkoscomm teuchoskokkoscompat
  teuchosremainder teuchosnumerics teuchoscomm teuchosparameterlist teuchosparser
  teuchoscore epetra epetraext ifpack aztecoo tpetraext tpetrainout tpetra kokkostsqr
  tpetraclassiclinalg tpetraclassicnodeapi tpetraclassic kokkoskernels ifpack2 triutils
  # repeat to solve issue we have on linux
  kokkosalgorithms
  teuchosparameterlist
)

# where the mains are located
set(MAINDIR ${CMAKE_CURRENT_SOURCE_DIR}/mains)

# headers for both eigen and kokkos
set(COMMONHEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/input_parser.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/utils.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/functors/advection_field_functors.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/functors/source_term_functors.hpp)

# headers just for eigen
set(EIGENHEADERS ${CMAKE_CURRENT_SOURCE_DIR}/adr2d_eigen.hpp)

#-------------------
# EIGEN-based exes
#-------------------
# Manufactured solution problem, Eigen, fom, rk4
set(SRCS_EIGEN_MS_FOM_RK4 ${MAINDIR}/main_eigen_ms_fom_rk4.cc)
add_executable(adr2d_eigen_ms_fom_rk4 ${SRCS_EIGEN_MS_FOM_RK4} ${COMMONHEADERS} ${EIGENHEADERS})
target_link_libraries(adr2d_eigen_ms_fom_rk4 ${TRILLIBNAMES})

# # Manufactured solution problem, Eigen, fom, bdf1
# set(SRCS_EIGEN_MS_FOM_BDF1 ${MAINDIR}/main_eigen_manuf_sol_fom_bdf1.cc)
# add_executable(adr2d_eigen_ms_fom_bdf1 ${SRCS_EIGEN_MS_FOM_BDF1} ${HEADERS})
# target_link_libraries(adr2d_eigen_ms_fom_bdf1 ${TRILLIBNAMES})

# # chemistry solution problem, Eigen, fom, rk4
# set(SRCS_EIGEN_CHEM_FOM_RK4 ${MAINDIR}/main_eigen_chem_fom_rk4.cc)
# add_executable(adr2d_eigen_chem_fom_rk4 ${SRCS_EIGEN_CHEM_FOM_RK4} ${HEADERS})
# target_link_libraries(adr2d_eigen_chem_fom_rk4 ${TRILLIBNAMES})

# # chemistry solution problem, Eigen, fom, rk4
# set(SRCS_EIGEN_CHEM_FOM_BDF1 ${MAINDIR}/main_eigen_chem_fom_bdf1.cc)
# add_executable(adr2d_eigen_chem_fom_bdf1 ${SRCS_EIGEN_CHEM_FOM_BDF1} ${HEADERS})
# target_link_libraries(adr2d_eigen_chem_fom_bdf1 ${TRILLIBNAMES})






#set(SOURCES_FOM_BDF1 ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.cc main_fom_bdf1.cc)
#set(SOURCES_ROM_LSPG ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.cc main_rom_lspg.cc)
#set(SOURCES_ROM_GALERKIN  ${CMAKE_CURRENT_SOURCE_DIR}/burgers1d_eigen.cc main_rom_galerkin.cc)
#add_executable(burgers1d_fom_bdf1 ${SOURCES_FOM_BDF1} ${HEADERS})
#add_executable(burgers1d_rom_lspg ${SOURCES_ROM_LSPG} ${HEADERS})
#add_executable(burgers1d_rom_galerkin ${SOURCES_ROM_GALERKIN} ${HEADERS})

#FILE(GLOB HEADERS RELATIVE
#  "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
