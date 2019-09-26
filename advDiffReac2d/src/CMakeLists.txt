
cmake_minimum_required(VERSION 3.11.0)
project(adr2d)

# every exe needs these includes and link dirs
include_directories(
  ${EIGEN_INCLUDE_DIR} ${TRILINOS_INCLUDE_DIR} ${PRESSIO_INCLUDE_DIR}
  )
link_directories(
  ${BLAS_LIB_DIR} ${LAPACK_LIB_DIR} ${TRILINOS_LIBRARY_DIR}
  )

# where the mains are located
set(MAINDIR ${CMAKE_CURRENT_SOURCE_DIR}/mains)

if(${HAVE_OMP})
  message("You enabled OpenMP")
  find_package(OpenMP)
endif()

#-------------------
# EIGEN exes
#-------------------
set(TRILLIBNAMESEIGEN
  kokkosalgorithms kokkoscontainers kokkoscore teuchoskokkoscomm teuchoskokkoscompat
  teuchosremainder teuchosnumerics teuchoscomm teuchosparameterlist teuchosparser teuchoscore
)

# eigen includes
set(EIGENINCDIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/functors
  ${CMAKE_CURRENT_SOURCE_DIR}/eigen)

# eigen Jacobian times phi
set(exeName adr2d_eigen_jphi_prod)
add_executable(${exeName} ${MAINDIR}/main_eigen_jphi_prod.cc)
target_include_directories(${exeName} BEFORE PUBLIC ${EIGENINCDIRS})
if(${HAVE_OMP})
  target_link_libraries(${exeName} ${TRILLIBNAMESEIGEN} OpenMP::OpenMP_CXX)
else()
  target_link_libraries(${exeName} ${TRILLIBNAMESEIGEN})
endif()


# eigen fom
set(exeName adr2d_eigen_fom)
add_executable(${exeName} ${MAINDIR}/main_eigen_fom.cc)
target_include_directories(${exeName} BEFORE PUBLIC ${EIGENINCDIRS})
if(${HAVE_OMP})
  target_link_libraries(${exeName} ${TRILLIBNAMESEIGEN} OpenMP::OpenMP_CXX)
else()
  target_link_libraries(${exeName} ${TRILLIBNAMESEIGEN})
endif()

# chemical problem, LSPG with full mesh bdf1
set(exeName adr2d_eigen_chem_lspg_full_mesh_bdf1)
add_executable(${exeName} ${MAINDIR}/main_eigen_chem_lspg_full_mesh_bdf1.cc)
target_include_directories(${exeName} BEFORE PUBLIC ${EIGENINCDIRS})
if(${HAVE_OMP})
  target_link_libraries(${exeName} ${TRILLIBNAMESEIGEN} OpenMP::OpenMP_CXX)
else()
  target_link_libraries(${exeName} ${TRILLIBNAMESEIGEN})
endif()

# # chemical problem, LSPG with SAMPLE mesh bdf1
# add_executable(adr2d_eigen_chem_lspg_sample_mesh_bdf1 ${MAINDIR}/main_eigen_chem_lspg_sample_mesh_bdf1.cc)
# target_include_directories(adr2d_eigen_chem_lspg_sample_mesh_bdf1 BEFORE PUBLIC ${EIGENINCDIRS})
# target_link_libraries(adr2d_eigen_chem_lspg_sample_mesh_bdf1 ${TRILLIBNAMES} OpenMP::OpenMP_CXX)


#-------------------
# KOKKOS exes
#-------------------
set(TRILLIBNAMESKOKKOS
  kokkosalgorithms kokkoscontainers kokkoscore teuchoskokkoscomm teuchoskokkoscompat
  teuchosremainder teuchosnumerics teuchoscomm teuchosparameterlist teuchosparser
  teuchoscore epetra epetraext ifpack aztecoo tpetraext tpetrainout tpetra kokkostsqr
  tpetraclassiclinalg tpetraclassicnodeapi tpetraclassic kokkoskernels ifpack2 triutils
  # repeat to solve issue we have on linux
  kokkosalgorithms teuchosparameterlist
)

# kokkos includes
set(KOKKOSINCDIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/functors
  ${CMAKE_CURRENT_SOURCE_DIR}/kokkos)

# kokkos Jacobian times phi
set(exeName adr2d_kokkos_jphi_prod)
add_executable(${exeName} ${MAINDIR}/main_kokkos_jphi_prod.cc)
target_include_directories(${exeName} BEFORE PUBLIC ${KOKKOSINCDIRS})
if(${HAVE_OMP})
  target_link_libraries(${exeName} ${TRILLIBNAMESKOKKOS} openblas OpenMP::OpenMP_CXX)
else()
  target_link_libraries(${exeName} ${TRILLIBNAMESKOKKOS} openblas)
endif()

# # kokkos fom
# add_executable(adr2d_kokkos_fom ${MAINDIR}/main_kokkos_fom.cc)
# target_include_directories(adr2d_kokkos_fom BEFORE PUBLIC ${KOKKOSINCDIRS})
# target_link_libraries(adr2d_kokkos_fom ${TRILLIBNAMES} openblas OpenMP::OpenMP_CXX)

# # chemical problem, LSPG with full mesh bdf1
# add_executable(adr2d_kokkos_chem_lspg_full_mesh_bdf1 ${MAINDIR}/main_kokkos_chem_lspg_full_mesh_bdf1.cc)
# target_include_directories(adr2d_kokkos_chem_lspg_full_mesh_bdf1 BEFORE PUBLIC ${KOKKOSINCDIRS})
# target_link_libraries(adr2d_kokkos_chem_lspg_full_mesh_bdf1 ${TRILLIBNAMES} openblas OpenMP::OpenMP_CXX)

# # chemical problem, LSPG with sample mesh bdf1
# add_executable(adr2d_kokkos_chem_lspg_sample_mesh_bdf1 ${MAINDIR}/main_kokkos_chem_lspg_sample_mesh_bdf1.cc)
# target_include_directories(adr2d_kokkos_chem_lspg_sample_mesh_bdf1 BEFORE PUBLIC ${KOKKOSINCDIRS})
# target_link_libraries(adr2d_kokkos_chem_lspg_sample_mesh_bdf1 ${TRILLIBNAMES} openblas OpenMP::OpenMP_CXX)
