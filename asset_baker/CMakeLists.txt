cmake_minimum_required (VERSION 3.8)
project ("asset_baker")

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

get_filename_component(EXT_DIR "../external" ABSOLUTE)
get_filename_component(SHARED_CPP "../shared_cpp" ABSOLUTE)
set(COMPRESSONATOR_DIR "C:/developer/repos/compressonator")

# Add source to this project's executable.
add_executable (asset_baker "asset_main.cpp")
target_include_directories(asset_baker PUBLIC
        ${EXT_DIR}
        ${EXT_DIR}/lz4
        ${EXT_DIR}/nlohmann
        ${EXT_DIR}/stb
        ${EXT_DIR}/tinygltf
        ${EXT_DIR}/tinyobjloader
        ${SHARED_CPP}
        ${SHARED_CPP}/assetlib
        ${COMPRESSONATOR_DIR}/cmp_compressonatorlib
)
target_link_libraries(
        asset_baker
        tinyobjloader
        stb_image
        tinygltf
        assetlib
        json
        lz4
        debug ${COMPRESSONATOR_DIR}/build/Debug_MD/x64/Compressonator_MDd.lib optimized ${COMPRESSONATOR_DIR}/build/Release_MD/x64/Compressonator_MD.lib)

# stb
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${EXT_DIR}/stb_image)

# nlohmann json
add_library(json INTERFACE)
target_include_directories(json INTERFACE ${EXT_DIR}/nlohmann )

# tinygltf
add_library(tinygltf STATIC)
target_sources(tinygltf PRIVATE
        ${EXT_DIR}/tinygltf/tiny_gltf.h
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
        ${EXT_DIR}/tinyobjloader/tiny_obj_loader.h
        ${EXT_DIR}/tinyobjloader/tiny_obj_loader.cc
)
target_include_directories(tinyobjloader PUBLIC ${EXT_DIR}/tinyobjloader)

# LZ4
add_library(lz4 STATIC)
target_sources(lz4 PRIVATE
        ${EXT_DIR}/lz4/lz4.h
        ${EXT_DIR}/lz4/lz4.c
)
target_include_directories(lz4 PUBLIC ${EXT_DIR}/lz4 )

# assetlib
set(ASSETLIB_DIR ${SHARED_CPP}/assetlib)
add_library (assetlib STATIC
        ${ASSETLIB_DIR}/asset_loader.cpp
        ${ASSETLIB_DIR}/cubemap_asset.cpp
        ${ASSETLIB_DIR}/texture_asset.cpp
        ${ASSETLIB_DIR}/model_asset.cpp
)
target_include_directories(assetlib PRIVATE
        ${ASSETLIB_INCL}
        ${EXT_DIR}
        ${EXT_DIR}/lz4
        ${EXT_DIR}/nlohmann
        ${SHARED_CPP}
)