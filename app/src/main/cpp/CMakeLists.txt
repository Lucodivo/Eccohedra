cmake_minimum_required(VERSION 3.4.1)
project(native-activity-blue)

# build native_app_glue as a static lib
set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
add_library(native_app_glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall -Werror")

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# TODO: Can I get the root of the Android project another way?
set(EXT_DIR "../../../../external")

add_library(native-activity-blue SHARED blue_scene.cpp)

# add include directories
set(NATIVE_SCENE_INCL ${ANDROID_NDK}/sources/android/native_app_glue ${EXT_DIR} ${EXT_DIR}/stb)
target_include_directories(native-activity-blue PRIVATE ${NATIVE_SCENE_INCL})

# add lib dependencies
set(NATIVE_SCENE_LIBS android native_app_glue EGL GLESv1_CM log)
target_link_libraries(native-activity-blue ${NATIVE_SCENE_LIBS})

#add_library(native-activity-red SHARED red_scene.cpp)
#target_include_directories(native-activity-red PRIVATE ${NATIVE_SCENE_INCL})
#target_link_libraries(native-activity-red ${NATIVE_SCENE_LIBS})
