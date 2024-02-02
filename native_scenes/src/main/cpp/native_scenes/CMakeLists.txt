add_library(gate-scene SHARED)
target_sources(gate-scene PRIVATE gate_scene.cpp)
set(COMMON_NATIVE_INCLUDE
        ${ANDROID_NATIVE_APP_GLUE_DIR}
        ${ANDROID_CPP}
        ${SHARED_CPP}
        ${SHARED_CPP}/assetlib
        ${EXT_DIR}/noopmath
)
set(COMMON_NATIVE_LIBS
        native_app_glue
        EGL
        GLESv3
        ${log-lib}
        ${android-lib}
        noopmath
        assetlib
)
target_include_directories(gate-scene PRIVATE
        ${COMMON_NATIVE_INCLUDE}
        ${EXT_DIR}
        ${EXT_DIR}/stb
        ${EXT_DIR}/nlohmann
        ${EXT_DIR}/lz4
        ${EXT_DIR}/noopmath
)
target_link_libraries(gate-scene
        ${COMMON_NATIVE_LIBS}
)

add_dependencies(gate-scene shaders-validation)