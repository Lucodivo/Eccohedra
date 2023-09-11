add_library(native-scene SHARED)
target_sources(native-scene PRIVATE
        native_scene.cpp)
target_include_directories(native-scene PRIVATE
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
target_link_libraries(native-scene
        native_app_glue
        EGL
        GLESv3
        ${log-lib}
        ${android-lib}
        noop_math
        assetlib
)

#[[
add_library(native-scene-red SHARED red_scene.cpp)
target_include_directories(native-scene-red PRIVATE ${NATIVE_SCENE_INCL})
target_link_libraries(native-scene-red ${NATIVE_SCENE_LIBS})
add_dependencies(native-scene Shaders)
]]
