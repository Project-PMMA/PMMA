# CMAKE general setup --------------------------------------------------
cmake_minimum_required(VERSION 3.10...3.27)
project(cpr_dependency LANGUAGES CXX)
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
    message(FATAL_ERROR "INSTALL_DIR is not defined. Please define INSTALL_DIR to specify where to install the built libraries.")
endif()

include(ExternalProject)

# External Dependency: CPR (C++ Requests) ------------------------------
ExternalProject_Add(cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.14.2
    GIT_SHALLOW TRUE
    CMAKE_GENERATOR Ninja
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}
        -DCMAKE_BUILD_TYPE=Debug
        -DBUILD_SHARED_LIBS=ON
        -DCPR_BUILD_TESTS=OFF
        # Instructs CPR to pull down curl/openssl automatically if missing
        -DCPR_BUILD_TESTS_SSL_DEPENDENCIES=ON
        -DCPR_FORCE_USE_SYSTEM_CURL=OFF
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Debug --parallel ${PARALLEL_CORES}
    INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --config Debug
)
