//
// Created by Connor on 6/22/2022.
//

#pragma once

#include <memory> // memset
#include <jni.h> // Jave Native Interface: Defines communication between Java and Cpp
#include <cassert> // asserts
#include <chrono>

#include <EGL/egl.h> // interface between OpenGL ES and underlying native platform window system
#include <GLES3/gl3.h> // OpenGL ES 3.0

#include <android/sensor.h> // Used for acquiring accelerometer sensor and corresponding event queue
#include <android/log.h> // Android logging
#include <android_native_app_glue.h> // Google's glue between android and native
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <dlfcn.h> // Android dynamic library utility functions

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>

const char* NATIVE_ACTIVITY_NAME = "native-activity-blue";
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, NATIVE_ACTIVITY_NAME, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, NATIVE_ACTIVITY_NAME, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, NATIVE_ACTIVITY_NAME, __VA_ARGS__))

#include "noop_types.h"

#include "noop_math.h"
#include "android_platform.cpp"
#include "util.h"
#include "shader_types_and_constants.h"
#include "vertex_attributes.h"
#include "vertex_attributes.cpp"
#include "model.h"
