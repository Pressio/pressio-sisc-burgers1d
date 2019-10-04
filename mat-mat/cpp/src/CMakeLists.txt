
cmake_minimum_required(VERSION 2.8.12)
project(burgers1dcpp)

include_directories(${EIGEN_INCLUDE_DIR})

link_directories(${BLAS_LIB_DIR})
add_definitions(-DEIGEN_USE_BLAS)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/input_parser.hpp)

add_executable(dgemm ${CMAKE_CURRENT_SOURCE_DIR}/main_dgemm.cc ${HEADERS})
target_link_libraries(dgemm openblas)

add_executable(spgemm ${CMAKE_CURRENT_SOURCE_DIR}/main_spgemm.cc ${HEADERS})
target_link_libraries(spgemm openblas)
