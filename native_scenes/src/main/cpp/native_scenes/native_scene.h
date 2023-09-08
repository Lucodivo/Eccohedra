//
// Created by Connor on 6/22/2022.
//

#pragma once

#include <memory> // memset
#include <jni.h> // Jave Native Interface: Defines communication between Java and Cpp
#include <cassert> // asserts
#include <chrono>
#include <math.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <EGL/egl.h> // interface between OpenGL ES and underlying native platform window system
#include <GLES3/gl32.h> // OpenGL ES 3.2

#include <android/sensor.h> // Used for acquiring accelerometer sensor and corresponding event queue
#include <android/log.h> // Android logging
#include <android_native_app_glue.h> // Google's glue between android and native
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <dlfcn.h> // Android dynamic library utility functions

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

const char* NATIVE_ACTIVITY_NAME = "native-activity-blue";
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, NATIVE_ACTIVITY_NAME, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, NATIVE_ACTIVITY_NAME, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, NATIVE_ACTIVITY_NAME, __VA_ARGS__))

#include "noop_types.h"

#include "../profiler/simplified_profiler.cpp"

#include "noop_math.h"
using namespace noop;

#include "asset_loader.h"
#include "android_platform.cpp"
#include "shader_types_and_constants.h"
#include "vertex_attributes.h"
#include "file_locations.h"
#include "save_file.h"
#include "util.h"
#include "textures.h"
#include "shader_program.h"
#include "model.h"
#include "camera.h"

#include "vertex_attributes.cpp"
#include "portal_scene.cpp"
