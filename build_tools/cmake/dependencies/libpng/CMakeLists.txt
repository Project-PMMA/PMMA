# CMAKE general setup --------------------------------------------------
cmake_minimum_required(VERSION 3.10...3.27)
project(libpng LANGUAGES CXX)
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

if (UNIX)
    set(ZLIB_LIBRARY ${INSTALL_DIR}/lib/libz.so)
elseif (WIN32)
    set(ZLIB_LIBRARY ${INSTALL_DIR}/lib/zlib.lib)
endif()

# External Dependency: libpng ------------------------------------------
ExternalProject_Add(libpng
    GIT_REPOSITORY https://github.com/glennrp/libpng.git
    GIT_TAG v1.6.49
    GIT_SHALLOW TRUE
    CMAKE_GENERATOR Ninja
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}
        -DCMAKE_BUILD_TYPE=Release
        -DBUILD_SHARED_LIBS=ON
        -DPNG_TESTS=OFf
        -DPNG_SHARED=ON
        -DPNG_STATIC=OFF
        -DZLIB_LIBRARY=${ZLIB_LIBRARY}
        -DZLIB_INCLUDE_DIR=${INSTALL_DIR}/include
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release --parallel ${PARALLEL_CORES}
    INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --config Release
)