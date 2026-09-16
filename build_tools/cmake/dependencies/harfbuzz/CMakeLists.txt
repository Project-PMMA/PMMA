# CMAKE general setup --------------------------------------------------
cmake_minimum_required(VERSION 3.10...3.27)
project(harfbuzz LANGUAGES CXX)
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

# External Dependency: harfbuzz ---------------------------------------- --
ExternalProject_Add(harfbuzz
    GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
    GIT_TAG 11.2.1
    GIT_SHALLOW TRUE
    # Meson setup command
    CONFIGURE_COMMAND meson setup <BINARY_DIR> <SOURCE_DIR>
        --prefix=${OUTPUT_DIR}
        --libdir=lib
        --buildtype=release
        --unity=on
        -Ddefault_library=shared
        -Dglib=disabled
        -Dfreetype=disabled
        -Dicu=disabled
        -Ddirectwrite=disabled
        -Dcoretext=disabled
        -Ddocs=disabled
        -Dtests=disabled
        -Dbenchmark=disabled
        -Dintrospection=disabled
        -Db_pch=true
        -Dutilities=disabled

    # Meson compile (uses Ninja)
    BUILD_COMMAND meson compile -C <BINARY_DIR> -j ${PARALLEL_CORES}

    # Meson install
    INSTALL_COMMAND meson install -C <BINARY_DIR>
)