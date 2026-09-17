cmake_minimum_required(VERSION 3.10...3.27)
cmake_policy(VERSION 3.22)

project(PMMA_Core LANGUAGES CXX)

# ---- Global settings -------------------------------------------------
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_PYTHON "Enable Python integration" OFF)

# ---- Paths -----------------------------------------------------------
if (NOT DEFINED WORKING_DIR)
    message(FATAL_ERROR "WORKING_DIR is not defined. Please set it to PMMA's root.")
endif()

set(LIB_NAME            PMMA_Core)
set(SRC_DIR             "${WORKING_DIR}/pmma/core/cpp_src")
set(INCLUDE_DIR         "${WORKING_DIR}/pmma/core/hpp_src")
set(EXTERN_INSTALL_DIR  "${WORKING_DIR}/pmma/extern")
set(LIB_OUTPUT_DIR      "${WORKING_DIR}/pmma/lib")

# ---- Sources ---------------------------------------------------------
set(CORE_SOURCES
    "${SRC_DIR}/PMMA.cpp"
    "${SRC_DIR}/Internal/Core/PMMA_Core.cpp"
    "${SRC_DIR}/Internal/Core/PMMA_Registry.cpp"


    "${SRC_DIR}/Animation/LinearAnimation.cpp"
    "${SRC_DIR}/Animation/RadialAnimation.cpp"

    "${SRC_DIR}/Events/ControllerEvents.cpp"
    "${SRC_DIR}/Events/KeyEvents.cpp"
    "${SRC_DIR}/Events/KeyPadEvents.cpp"
    "${SRC_DIR}/Events/MouseEvents.cpp"
    "${SRC_DIR}/Events/WindowEvents.cpp"

    "${SRC_DIR}/Graphics/Shader.cpp"

    "${SRC_DIR}/Internal/AnimationManager.cpp"
    "${SRC_DIR}/Internal/PowerSavingManager.cpp"
    "${SRC_DIR}/Internal/LoggingManager.cpp"
    "${SRC_DIR}/Internal/Internal.cpp"
    "${SRC_DIR}/Internal/SplashScreen.cpp"
    "${SRC_DIR}/Internal/NetworkingManager.cpp"
    "${SRC_DIR}/Internal/TerminalColorChecker.cpp"
    "${SRC_DIR}/Internal/SavedDataManager.cpp"

    "${SRC_DIR}/Internal/Events/EventsManager.cpp"
    "${SRC_DIR}/Internal/Events/InternalEvents.cpp"

    "${SRC_DIR}/Internal/Rendering/Core2D/RenderPipelineManager.cpp"
    "${SRC_DIR}/Internal/Rendering/Core2D/RenderPipelineInstance.cpp"
    "${SRC_DIR}/Internal/Rendering/Core2D/CompressedTextureInstance.cpp"
    "${SRC_DIR}/Internal/Rendering/Core2D/GeneratedTextureManager.cpp"

    "${SRC_DIR}/Internal/Utility/CPU_FeatureSetUtils.cpp"
    "${SRC_DIR}/Internal/Utility/FontUtils.cpp"

    "${SRC_DIR}/Rendering/TextRenderer.cpp"

    "${SRC_DIR}/Rendering/Shapes2D/RectangleShape.cpp"
    "${SRC_DIR}/Rendering/Shapes2D/RadialPolygonShape.cpp"
    "${SRC_DIR}/Rendering/Shapes2D/PixelShape.cpp"
    "${SRC_DIR}/Rendering/Shapes2D/LineShape.cpp"
    "${SRC_DIR}/Rendering/Shapes2D/ArcShape.cpp"

    "${SRC_DIR}/Noise/FractalBrownianMotion.cpp"
    "${SRC_DIR}/Noise/PerlinNoise_BASE.cpp"

    "${SRC_DIR}/Types/Color.cpp"
    "${SRC_DIR}/Types/Angle.cpp"
    "${SRC_DIR}/Types/Proportion.cpp"
    "${SRC_DIR}/Types/Coordinate.cpp"
    "${SRC_DIR}/Types/Size.cpp"
    "${SRC_DIR}/Types/CompressedTexture.cpp"
    "${SRC_DIR}/Types/GeneratedTexture.cpp"

    "${SRC_DIR}/Maths_BASE.cpp"
    "${SRC_DIR}/Display.cpp"
    "${SRC_DIR}/General.cpp"
    "${SRC_DIR}/Logger.cpp"
    "${SRC_DIR}/Passport.cpp"
    "${SRC_DIR}/Random.cpp"
)

# ---- Shared Compiler Params ------------------------------------------

if (MSVC)
    set(COMMON_COMPILE_OPTIONS
        /fp:fast /GL /GF
        /GS- /std:c++20 /wd4551 /wd4251
        /Oi /Ob2 /Zc:__cplusplus /Zc:preprocessor /MP
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(COMMON_COMPILE_OPTIONS
        -O3 -ffast-math -funroll-loops
        -fstrict-aliasing -fomit-frame-pointer
        -std=c++20 -mtune=generic
        -finline-functions -flto -ftree-vectorize
    )
else()
    set(ZSTD_C_FLAGS "")
    set(ZSTD_LINKER_FLAGS "")
endif()

# ---- SIMD subcomponents ----------------------------------------------
if(NOT APPLE)
    # AVX2
    add_library(AVX2_obj OBJECT
	    "${SRC_DIR}/Maths_AVX2.cpp"
	    "${SRC_DIR}/Noise/PerlinNoise_AVX2.cpp")

    if (MSVC)
        target_compile_options(AVX2_obj PRIVATE
            ${COMMON_COMPILE_OPTIONS}
            $<$<CONFIG:Debug>:/Od /RTC1>
            $<$<CONFIG:Release>:/O2>
            /arch:AVX2)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	    target_compile_options(AVX2_obj PRIVATE
            ${COMMON_COMPILE_OPTIONS}
            $<$<CONFIG:Debug>:-O0>
            $<$<CONFIG:Release>:-O3>
            -mavx2 -mfma)
    endif()

    target_include_directories(AVX2_obj PRIVATE
	    ${INCLUDE_DIR}
	    ${EXTERN_INSTALL_DIR}/include
	    ${EXTERN_INSTALL_DIR}/include/freetype2)
    target_compile_definitions(AVX2_obj PRIVATE PMMA_CORE_EXPORTS BX_CONFIG_DEBUG=0)

    if (MSVC)
        target_compile_definitions(AVX2_obj PRIVATE NOMINMAX)
    endif()

    # AVX-512
    add_library(AVX512_obj OBJECT
	    "${SRC_DIR}/Maths_AVX512.cpp"
	    "${SRC_DIR}/Noise/PerlinNoise_AVX512.cpp")

    if (MSVC)
        target_compile_options(AVX512_obj PRIVATE
            ${COMMON_COMPILE_OPTIONS}
            $<$<CONFIG:Debug>:/Od /RTC1>
            $<$<CONFIG:Release>:/O2>
            /arch:AVX512)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	    target_compile_options(AVX512_obj PRIVATE
            ${COMMON_COMPILE_OPTIONS}
            $<$<CONFIG:Debug>:-O0>
            $<$<CONFIG:Release>:-O3>
            -mavx512f -mavx512dq)
    endif()

    target_include_directories(AVX512_obj PRIVATE
	    ${INCLUDE_DIR}
	    ${EXTERN_INSTALL_DIR}/include
        ${EXTERN_INSTALL_DIR}/include/freetype2)

    target_compile_definitions(AVX512_obj PRIVATE PMMA_CORE_EXPORTS BX_CONFIG_DEBUG=0)

    if (MSVC)
        target_compile_definitions(AVX512_obj PRIVATE NOMINMAX)
    endif()

    add_library(${LIB_NAME} SHARED ${CORE_SOURCES} $<TARGET_OBJECTS:AVX2_obj> $<TARGET_OBJECTS:AVX512_obj>)
else()
    message(STATUS "Skipping AVX extensions on unsupported platform")
    add_library(AVX_Filler_obj OBJECT "${SRC_DIR}/AVX_Filler.cpp")

    if (MSVC)
        target_compile_options(AVX_Filler_obj PRIVATE
            ${COMMON_COMPILE_OPTIONS}
            $<$<CONFIG:Debug>:/Od /RTC1>
            $<$<CONFIG:Release>:/O2>)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	    target_compile_options(AVX_Filler_obj PRIVATE
            ${COMMON_COMPILE_OPTIONS}
            $<$<CONFIG:Debug>:-O0>
            $<$<CONFIG:Release>:-O3>)
    endif()

    target_include_directories(AVX_Filler_obj PRIVATE
	    ${INCLUDE_DIR}
	    ${EXTERN_INSTALL_DIR}/include
	    ${EXTERN_INSTALL_DIR}/include/freetype2)

    target_compile_definitions(AVX_Filler_obj PRIVATE PMMA_CORE_EXPORTS BX_CONFIG_DEBUG=0)

    add_library(${LIB_NAME} SHARED ${CORE_SOURCES} $<TARGET_OBJECTS:AVX_Filler_obj>)
endif()

# ---- IPO / LTO ---------------------------------------------------------------
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_msg)
if(lto_supported)
    set_target_properties(${LIB_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# ---- Includes / Defines ------------------------------------------------------
target_compile_definitions(${LIB_NAME} PRIVATE PMMA_CORE_EXPORTS BX_CONFIG_DEBUG=0)

if (MSVC)
    target_compile_definitions(${LIB_NAME} PRIVATE
        NOMINMAX
    )
endif()


target_include_directories(${LIB_NAME} PRIVATE
    ${INCLUDE_DIR}
    ${EXTERN_INSTALL_DIR}/include
    ${EXTERN_INSTALL_DIR}/include/freetype2)

# ---- Link directories (extern install prefix variations) ---------------------
file(GLOB LIBDIRS "${EXTERN_INSTALL_DIR}/lib*")
foreach(libdir ${LIBDIRS})
    if(IS_DIRECTORY "${libdir}")
	    message(STATUS "Using link directory: ${libdir}")
	    target_link_directories(${LIB_NAME} PRIVATE ${libdir})
    endif()
endforeach()

# ---- Python (optional) -------------------------------------------------------
if(USE_PYTHON)
    find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
    target_compile_definitions(${LIB_NAME} PRIVATE USE_PYTHON)
    target_include_directories(${LIB_NAME} PRIVATE ${Python3_INCLUDE_DIRS})
    target_link_libraries(${LIB_NAME} PRIVATE ${Python3_LIBRARIES})
endif()

# Platform toolkits (glfw kept as your window/input layer)
# Note: We intentionally do NOT find_package(OpenGL) here—BGFX selects renderers at runtime.
if (WIN32)
    target_compile_options(${LIB_NAME} PRIVATE
        ${COMMON_COMPILE_OPTIONS}
        $<$<CONFIG:Debug>:/Od /RTC1>
        $<$<CONFIG:Release>:/O2>)

    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LTCG")

    target_link_libraries(${LIB_NAME} PRIVATE
	    glfw3dll bc7enc bgfx bx bimg bimg_encode freetype libpng16 zlib harfbuzz zstd cpr)
elseif(UNIX)
    target_compile_options(${LIB_NAME} PRIVATE
        ${COMMON_COMPILE_OPTIONS}
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Release>:-O3>)

    target_link_libraries(${LIB_NAME} PRIVATE
	    glfw bc7enc bgfx bx bimg bimg_encode freetype png z harfbuzz zstd pthread dl cpr)

    target_link_options(${LIB_NAME} PUBLIC -flto=auto)
endif()

# ---- Output directories ------------------------------------------------------
if(CMAKE_CONFIGURATION_TYPES) # Multi-config (MSVC, Xcode)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
	    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
	    set_target_properties(${LIB_NAME} PROPERTIES
	        RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${LIB_OUTPUT_DIR}
	        LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${LIB_OUTPUT_DIR}
	         ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${LIB_OUTPUT_DIR})
    endforeach()
else() # Single-config (Makefiles, Ninja)
    set_target_properties(${LIB_NAME} PROPERTIES
	    RUNTIME_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}
	    LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}
	    ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})
endif()

# ---- Notes -------------------------------------------------------------------
# AVX2 MSVC: /arch:AVX2
# AVX2 GCC/Clang: -mavx2 -mfma
# AVX-512 MSVC: /arch:AVX512
# AVX-512 GCC/Clang: -mavx512f -mavx512dq