cmake_minimum_required(VERSION 3.10...3.27)

project(bc7enc LANGUAGES CXX)

cmake_policy(VERSION 3.22)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ----------------------------------------------------------------------
# Configuration
# ----------------------------------------------------------------------

if(NOT DEFINED OUTPUT_DIR)
    message(FATAL_ERROR
        "OUTPUT_DIR is not defined."
    )
endif()

if(NOT DEFINED INSTALL_DIR)
    message(FATAL_ERROR
        "INSTALL_DIR is not defined."
    )
endif()

# ----------------------------------------------------------------------
# Fetch bc7enc source
# ----------------------------------------------------------------------

include(FetchContent)

FetchContent_Declare(
    bc7enc_source
    GIT_REPOSITORY https://github.com/richgel999/bc7enc_rdo.git
    GIT_TAG b9438627eef73a1157e84201b6fa6eb2ffd6d9f0
    GIT_SHALLOW TRUE
)

FetchContent_Populate(bc7enc_source)

message(STATUS "bc7enc source: ${bc7enc_source_SOURCE_DIR}")

# ----------------------------------------------------------------------
# Build BC7 encoder + decoder
# ----------------------------------------------------------------------

add_library(bc7enc SHARED

    # Upstream encoder
    "${bc7enc_source_SOURCE_DIR}/bc7enc.cpp"

    # Upstream decoder
    "${bc7enc_source_SOURCE_DIR}/bc7decomp.cpp"

    # PMMA encoder wrapper
    "${CMAKE_CURRENT_SOURCE_DIR}/bc7enc_wrapper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bc7enc_wrapper.hpp"

    # PMMA decoder wrapper
    "${CMAKE_CURRENT_SOURCE_DIR}/bc7dec_wrapper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/bc7dec_wrapper.hpp"
)

# ----------------------------------------------------------------------
# Include directories
# ----------------------------------------------------------------------

target_include_directories(bc7enc PUBLIC
    "${bc7enc_source_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# ----------------------------------------------------------------------
# Export wrapper functions
# ----------------------------------------------------------------------

target_compile_definitions(bc7enc PRIVATE
    BC7ENC_EXPORTS
)

# ----------------------------------------------------------------------
# Optimization
# ----------------------------------------------------------------------

if(MSVC)

    target_compile_options(bc7enc PRIVATE
        /fp:fast /GL /GF
        /GS- /std:c++20 /wd4551 /wd4251
        /Oi /Ob2 /Zc:__cplusplus /Zc:preprocessor /MP
    )

else()

    target_compile_options(bc7enc PRIVATE
        -O3 -ffast-math -funroll-loops
        -fstrict-aliasing -fomit-frame-pointer
        -std=c++20 -mtune=generic
        -finline-functions -flto -ftree-vectorize
    )

endif()

# ----------------------------------------------------------------------
# Output directories
# ----------------------------------------------------------------------

set_target_properties(bc7enc PROPERTIES
    UNITY_BUILD OFF

    RUNTIME_OUTPUT_DIRECTORY
        "${OUTPUT_DIR}/bin"

    LIBRARY_OUTPUT_DIRECTORY
        "${OUTPUT_DIR}/lib"

    ARCHIVE_OUTPUT_DIRECTORY
        "${OUTPUT_DIR}/lib"

    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${OUTPUT_DIR}/bin"

    LIBRARY_OUTPUT_DIRECTORY_RELEASE
        "${OUTPUT_DIR}/lib"

    ARCHIVE_OUTPUT_DIRECTORY_RELEASE
        "${OUTPUT_DIR}/lib"
)

# ----------------------------------------------------------------------
# Copy public headers into dependency output
# ----------------------------------------------------------------------

add_custom_command(
    TARGET bc7enc
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${OUTPUT_DIR}/include"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/bc7enc_wrapper.hpp"
        "${OUTPUT_DIR}/include/bc7enc_wrapper.hpp"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/bc7dec_wrapper.hpp"
        "${OUTPUT_DIR}/include/bc7dec_wrapper.hpp"
)