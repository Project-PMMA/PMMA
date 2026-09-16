# CMAKE general setup --------------------------------------------------
cmake_minimum_required(VERSION 3.10...3.27)
project(zstd LANGUAGES CXX)
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

# Determine safe, high-performance C flags based on the active compiler
if (MSVC)
    # Safe MSVC performance options for ZSTD (No C++ flags, no /fp:fast)
    set(ZSTD_C_FLAGS "/O2 /Ob2 /Oi /GL /arch:AVX2")
    set(ZSTD_LINKER_FLAGS "/LTCG")
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    # Safe GCC/Clang performance options for ZSTD (No C++ flags, no -ffast-math)
    set(ZSTD_C_FLAGS "-O3 -fstrict-aliasing -fomit-frame-pointer -flto -funroll-loops -ftree-vectorize")
    set(ZSTD_LINKER_FLAGS "-flto")
else()
    # Fallback to standard Release flags for unknown compilers
    set(ZSTD_C_FLAGS "")
    set(ZSTD_LINKER_FLAGS "")
endif()

# External Dependency: zstd --------------------------------------------
ExternalProject_Add(zstd
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG v1.5.7
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR build/cmake
    CMAKE_GENERATOR Ninja
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}
        -DCMAKE_BUILD_TYPE=Release
        -DBUILD_SHARED_LIBS=ON
        -DZSTD_ENABLE_TESTS=OFF
        -DZSTD_BUILD_SHARED=ON
        -DZSTD_BUILD_STATIC=OFF
        -DZSTD_BUILD_EXAMPLES=OFF
        -DZSTD_BUILD_PROGRAMS=OFF

        -DCMAKE_C_FLAGS=${ZSTD_C_FLAGS}
        -DCMAKE_SHARED_LINKER_FLAGS=${ZSTD_LINKER_FLAGS}
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config Release --parallel ${PARALLEL_CORES}
    INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --config Release
)