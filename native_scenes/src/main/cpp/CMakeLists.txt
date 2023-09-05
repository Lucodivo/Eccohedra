cmake_minimum_required(VERSION 3.4.1)
project(native-activities)

# build native_app_glue as a static lib
set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17 -Wall -Wno-unused-variable -Wno-null-dereference -Wno-missing-braces -Wno-sometimes-uninitialized -Werror")

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# TODO: Can I get the root of the Android project another way?
set(EXT_DIR "../../../../external")

add_library(native-scene SHARED native_scene.cpp)

# add include directories
set(NATIVE_SCENE_INCL ${ANDROID_NDK}/sources/android/native_app_glue ${EXT_DIR} ${EXT_DIR}/stb ${EXT_DIR}/nlohmann)
target_include_directories(native-scene PRIVATE ${NATIVE_SCENE_INCL})

find_library(log-lib log)
find_library(android-lib android)

# add lib dependencies
set(NATIVE_SCENE_LIBS native_app_glue EGL GLESv3 ${log-lib} ${android-lib})
target_link_libraries(native-scene ${NATIVE_SCENE_LIBS})

#add_library(native-scene-red SHARED red_scene.cpp)
#target_include_directories(native-scene-red PRIVATE ${NATIVE_SCENE_INCL})
#target_link_libraries(native-scene-red ${NATIVE_SCENE_LIBS})
