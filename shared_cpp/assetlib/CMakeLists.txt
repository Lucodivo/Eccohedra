# Add source to this project's executable.
add_library (assetlib STATIC
        "asset_loader.cpp"
        "texture_asset.cpp"
        "cubemap_asset.cpp"
)

set(ASSETLIB_INCL
        ${ANDROID_NDK}/sources/android/native_app_glue
        ${EXT_DIR}
        ${EXT_DIR}/lz4
        ${EXT_DIR}/nlohmann
        ${ANDROID_CPP_ROOT}/noop_math
        ${ANDROID_CPP_ROOT}
)
target_include_directories(assetlib PRIVATE ${ASSETLIB_INCL})