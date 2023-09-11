cmake_minimum_required (VERSION 3.8)
project ("native_scenes")

set(CMAKE_CXX_STANDARD 17)

set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17 -Wall -Wno-unused-variable -Wno-unused-function -Wno-format -Wno-null-dereference -Wno-missing-braces -Wno-sometimes-uninitialized -Werror -Wno-uninitialized")

get_filename_component(ANDROID_ROOT "../" ABSOLUTE)
get_filename_component(ANDROID_CPP "src/main/cpp/" ABSOLUTE)
get_filename_component(ANDROID_NATIVE_APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue ABSOLUTE)
set(EXT_DIR ${ANDROID_ROOT}/external)
set(SHARED_CPP ${ANDROID_ROOT}/shared_cpp/)

find_library(log-lib log)
find_library(android-lib android)

# build native_app_glue as a static lib
add_library(native_app_glue STATIC ${ANDROID_NATIVE_APP_GLUE_DIR}/android_native_app_glue.c)
# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# stb
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${EXT_DIR}/stb_image)

# nlohmann json
add_library(json INTERFACE)
target_include_directories(json INTERFACE ${EXT_DIR}/nlohmann )

# tinygltf
add_library(tinygltf STATIC)
target_sources(tinygltf PRIVATE
        ${EXT_DIR}/tinygltf/tiny_gltf.cc
)
target_include_directories(tinygltf PUBLIC
        ${EXT_DIR}/tinygltf
        ${EXT_DIR}/stb
        ${EXT_DIR}/nlohmann
)

# tinyobjloader
add_library(tinyobjloader STATIC)
target_sources(tinyobjloader PRIVATE
        ${EXT_DIR}/tinyobjloader/tiny_obj_loader.cc
)
target_include_directories(tinyobjloader PUBLIC ${EXT_DIR}/tinyobjloader)

# LZ4
add_library(lz4 STATIC)
target_sources(lz4 PRIVATE
        ${EXT_DIR}/lz4/lz4.c
)
target_include_directories(lz4 PUBLIC ${EXT_DIR}/lz4 )

# noop_math
add_library(noop_math STATIC)
target_sources(noop_math PRIVATE
        ${SHARED_CPP}/noop_math/noop_math.cpp
)
target_include_directories(noop_math PRIVATE ${SHARED_CPP})

# assetlib
add_library (assetlib STATIC)
target_sources(assetlib PRIVATE
        ${SHARED_CPP}/assetlib/asset_loader.cpp
        ${SHARED_CPP}/assetlib/cubemap_asset.cpp
)
target_include_directories(assetlib PRIVATE
        ${EXT_DIR}/lz4
        ${EXT_DIR}/nlohmann
        ${SHARED_CPP}
        ${ANDROID_NATIVE_APP_GLUE_DIR}
)
target_link_libraries(assetlib ${log-lib} ${android-lib} lz4 json)

# Android library CMake directories
add_subdirectory(src/main/cpp/native_scenes)

#[[ Using CMake to validate glsl shader files
find_package(Vulkan REQUIRED)
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

## find all the shader files under the shaders folder
file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${PROJECT_SOURCE_DIR}/src/assets/shaders/*.frag"
        "${PROJECT_SOURCE_DIR}/src/assets/shaders/*.vert"
        "${PROJECT_SOURCE_DIR}/src/assets/shaders/*.comp"
)

## iterate each shader
foreach(GLSL ${GLSL_SOURCE_FILES})
    message(STATUS "BUILDING SHADER")
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/src/assets/shaders/spv/${FILE_NAME}.spv")
    message(STATUS ${GLSL})
    ##execute glslang command to compile that specific shader
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)
]]