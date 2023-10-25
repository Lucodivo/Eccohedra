add_library(gate-scene SHARED)
target_sources(gate-scene PRIVATE
        gate_scene.cpp)
target_include_directories(gate-scene PRIVATE
        ${ANDROID_NATIVE_APP_GLUE_DIR}
        ${ANDROID_CPP}
        ${EXT_DIR}
        ${EXT_DIR}/stb
        ${EXT_DIR}/nlohmann
        ${EXT_DIR}/lz4
        ${SHARED_CPP}
        ${SHARED_CPP}/assetlib
        ${SHARED_CPP}/noop_math
)
target_link_libraries(gate-scene
        native_app_glue
        EGL
        GLESv3
        ${log-lib}
        ${android-lib}
        noop_math
        assetlib
)

add_dependencies(gate-scene shaders-validation)

#[[
add_library(gate-scene-red SHARED red_scene.cpp)
target_include_directories(gate-scene-red PRIVATE ${NATIVE_SCENE_INCL})
target_link_libraries(gate-scene-red ${NATIVE_SCENE_LIBS})
]]
