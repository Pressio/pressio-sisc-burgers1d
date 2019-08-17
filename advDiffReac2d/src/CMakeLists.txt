
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

#-------------------
# EIGEN exes
#-------------------
# where the mains are located
set(EIGENMAINDIR ${CMAKE_CURRENT_SOURCE_DIR}/mains_eigen)

# eigen includes
set(EIGENINCDIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/functors)

# # Manufactured solution problem, fom, rk4
# add_executable(adr2d_eigen_ms_fom_rk4 ${EIGENMAINDIR}/main_eigen_ms_fom_rk4.cc)
# target_include_directories(adr2d_eigen_ms_fom_rk4 BEFORE PUBLIC ${EIGENINCDIRS})
# target_link_libraries(adr2d_eigen_ms_fom_rk4 ${TRILLIBNAMES})

# # Manufactured solution problem, fom, bdf1
# add_executable(adr2d_eigen_ms_fom_bdf1 ${EIGENMAINDIR}/main_eigen_ms_fom_bdf1.cc)
# target_include_directories(adr2d_eigen_ms_fom_bdf1 BEFORE PUBLIC ${EIGENINCDIRS})
# target_link_libraries(adr2d_eigen_ms_fom_bdf1 ${TRILLIBNAMES})

# # chemical problem, fom, rk4
# add_executable(adr2d_eigen_chem_fom_rk4 ${EIGENMAINDIR}/main_eigen_chem_fom_rk4.cc)
# target_include_directories(adr2d_eigen_chem_fom_rk4 BEFORE PUBLIC ${EIGENINCDIRS})
# target_link_libraries(adr2d_eigen_chem_fom_rk4 ${TRILLIBNAMES})

# # chemical problem, fom, bdf1
# add_executable(adr2d_eigen_chem_fom_bdf1 ${EIGENMAINDIR}/main_eigen_chem_fom_bdf1.cc)
# target_include_directories(adr2d_eigen_chem_fom_bdf1 BEFORE PUBLIC ${EIGENINCDIRS})
# target_link_libraries(adr2d_eigen_chem_fom_bdf1 ${TRILLIBNAMES})


# eigen fom
add_executable(adr2d_eigen_fom ${EIGENMAINDIR}/main_eigen_fom.cc)
target_include_directories(adr2d_eigen_fom BEFORE PUBLIC ${EIGENINCDIRS})
target_link_libraries(adr2d_eigen_fom ${TRILLIBNAMES})

# chemical problem, LSPG with full mesh bdf1
add_executable(adr2d_eigen_chem_lspg_full_mesh_bdf1 ${EIGENMAINDIR}/main_eigen_chem_lspg_full_mesh_bdf1.cc)
target_include_directories(adr2d_eigen_chem_lspg_full_mesh_bdf1 BEFORE PUBLIC ${EIGENINCDIRS})
target_link_libraries(adr2d_eigen_chem_lspg_full_mesh_bdf1 ${TRILLIBNAMES})

#-------------------
# KOKKOS exes
#-------------------
# where the mains are located
set(KOKKOSMAINDIR ${CMAKE_CURRENT_SOURCE_DIR}/mains_kokkos)

# kokkos includes
set(KOKKOSINCDIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/functors)

# chemical problem, fom, rk4
add_executable(adr2d_kokkos_chem_fom_rk4 ${KOKKOSMAINDIR}/main_kokkos_chem_fom_rk4.cc)
target_include_directories(adr2d_kokkos_chem_fom_rk4 BEFORE PUBLIC ${KOKKOSINCDIRS})
target_link_libraries(adr2d_kokkos_chem_fom_rk4 ${TRILLIBNAMES} openblas)

# chemical problem, LSPG with full mesh bdf1
add_executable(adr2d_kokkos_chem_lspg_full_mesh_bdf1 ${KOKKOSMAINDIR}/main_kokkos_chem_lspg_full_mesh_bdf1.cc)
target_include_directories(adr2d_kokkos_chem_lspg_full_mesh_bdf1 BEFORE PUBLIC ${KOKKOSINCDIRS})
target_link_libraries(adr2d_kokkos_chem_lspg_full_mesh_bdf1 ${TRILLIBNAMES} openblas)
