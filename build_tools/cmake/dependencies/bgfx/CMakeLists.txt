# CMAKE general setup --------------------------------------------------
cmake_minimum_required(VERSION 3.10...3.27)
project(bgfx LANGUAGES CXX)
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

# External Dependency: bgfx --------------------------------------------

set(CMAKE_CXX_FLAGS "")

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /std:c++20 /DBX_CONFIG_DEBUG=0")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -DBX_CONFIG_DEBUG=0")
endif()


ExternalProject_Add(bgfx
    GIT_REPOSITORY https://github.com/bkaradzic/bgfx.cmake.git
    GIT_TAG v1.143.9248-540
    GIT_SHALLOW TRUE
    CMAKE_GENERATOR Ninja
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_CXX_STANDARD=17
        -DCMAKE_CXX_STANDARD_REQUIRED=ON
        -DCMAKE_CXX_EXTENSIONS=OFF
        -DBX_AMALGAMATED=ON
        -DBGFX_AMALGAMATED=ON
        -DBGFX_BUILD_EXAMPLES=OFF
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release --parallel ${PARALLEL_CORES}
    INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --config Release
)