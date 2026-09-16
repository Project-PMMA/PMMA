# CMAKE general setup --------------------------------------------------
cmake_minimum_required(VERSION 3.10...3.27)
project(zlib LANGUAGES CXX)
cmake_policy(VERSION 3.22)

# CMAKE configuration --------------------------------------------------
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(ProcessorCount)
ProcessorCount(NPROC)
if(NOT DEFINED PARALLEL_CORES)
    set(PARALLEL_CORES ${NPROC})
endif()

if (NOT DEFINED OUTPUT_DIR)
    message(FATAL_ERROR "OUTPUT_DIR is not defined. Please define OUTPUT_DIR to specify where to store the built libraries.")
endif()

if (NOT DEFINED INSTALL_DIR)
    message(FATAL_ERROR "INSTALL_DIR is not defined. Please define OUTPUT_DIR to specify where to install the built libraries.")
endif()

include(ExternalProject)

# External Dependency: zlib --------------------------------------------
ExternalProject_Add(zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.3.1
    GIT_SHALLOW TRUE
    CMAKE_GENERATOR Ninja
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}
        -DCMAKE_BUILD_TYPE=Release
        -DBUILD_SHARED_LIBS=ON
        -DZLIB_ENABLE_TESTS=OFF
        -DZLIB_BUILD_SHARED=ON
        -DZLIB_BUILD_STATIC=OFF
        -DZLIB_BUILD_EXAMPLES=OFF
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release --parallel ${PARALLEL_CORES}
    INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --config Release
)