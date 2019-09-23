
cmake_minimum_required(VERSION 2.8.12)
project(adr2d)

# every exe needs these includes and link dirs
include_directories(
  ${EIGEN_INCLUDE_DIR} ${TRILINOS_INCLUDE_DIR} ${PRESSIO_INCLUDE_DIR}
  )
link_directories(
  ${BLAS_LIB_DIR} ${LAPACK_LIB_DIR} ${TRILINOS_LIBRARY_DIR}
  )

set(TRILLIBNAMES
  kokkosalgorithms kokkoscontainers kokkoscore teuchoskokkoscomm teuchoskokkoscompat
  teuchosremainder teuchosnumerics teuchoscomm teuchosparameterlist teuchosparser
  teuchoscore epetra epetraext ifpack aztecoo tpetraext tpetrainout tpetra kokkostsqr
  tpetraclassiclinalg tpetraclassicnodeapi tpetraclassic kokkoskernels ifpack2 triutils
  # repeat to solve issue we have on linux
  kokkosalgorithms teuchosparameterlist
)

# where the mains are located
set(MAINDIR ${CMAKE_CURRENT_SOURCE_DIR}/mains)

#-------------------
# EIGEN exes
#-------------------

find_package(OpenMP)

# eigen includes
set(EIGENINCDIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/functors
  ${CMAKE_CURRENT_SOURCE_DIR}/eigen)

# eigen fom
add_executable(adr2d_eigen_fom ${MAINDIR}/main_eigen_fom.cc)
target_include_directories(adr2d_eigen_fom BEFORE PUBLIC ${EIGENINCDIRS})
target_link_libraries(adr2d_eigen_fom ${TRILLIBNAMES} OpenMP::OpenMP_CXX)

# chemical problem, LSPG with full mesh bdf1
add_executable(adr2d_eigen_chem_lspg_full_mesh_bdf1 ${MAINDIR}/main_eigen_chem_lspg_full_mesh_bdf1.cc)
target_include_directories(adr2d_eigen_chem_lspg_full_mesh_bdf1 BEFORE PUBLIC ${EIGENINCDIRS})
target_link_libraries(adr2d_eigen_chem_lspg_full_mesh_bdf1 ${TRILLIBNAMES} OpenMP::OpenMP_CXX)

# chemical problem, LSPG with SAMPLE mesh bdf1
add_executable(adr2d_eigen_chem_lspg_sample_mesh_bdf1 ${MAINDIR}/main_eigen_chem_lspg_sample_mesh_bdf1.cc)
target_include_directories(adr2d_eigen_chem_lspg_sample_mesh_bdf1 BEFORE PUBLIC ${EIGENINCDIRS})
target_link_libraries(adr2d_eigen_chem_lspg_sample_mesh_bdf1 ${TRILLIBNAMES})

#-------------------
# KOKKOS exes
#-------------------
# kokkos includes
set(KOKKOSINCDIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/functors
  ${CMAKE_CURRENT_SOURCE_DIR}/kokkos)

# kokkos fom
add_executable(adr2d_kokkos_fom ${MAINDIR}/main_kokkos_fom.cc)
target_include_directories(adr2d_kokkos_fom BEFORE PUBLIC ${KOKKOSINCDIRS})
target_link_libraries(adr2d_kokkos_fom ${TRILLIBNAMES} openblas)

# chemical problem, LSPG with full mesh bdf1
add_executable(adr2d_kokkos_chem_lspg_full_mesh_bdf1 ${MAINDIR}/main_kokkos_chem_lspg_full_mesh_bdf1.cc)
target_include_directories(adr2d_kokkos_chem_lspg_full_mesh_bdf1 BEFORE PUBLIC ${KOKKOSINCDIRS})
target_link_libraries(adr2d_kokkos_chem_lspg_full_mesh_bdf1 ${TRILLIBNAMES} openblas)

# chemical problem, LSPG with sample mesh bdf1
add_executable(adr2d_kokkos_chem_lspg_sample_mesh_bdf1 ${MAINDIR}/main_kokkos_chem_lspg_sample_mesh_bdf1.cc)
target_include_directories(adr2d_kokkos_chem_lspg_sample_mesh_bdf1 BEFORE PUBLIC ${KOKKOSINCDIRS})
target_link_libraries(adr2d_kokkos_chem_lspg_sample_mesh_bdf1 ${TRILLIBNAMES} openblas)
