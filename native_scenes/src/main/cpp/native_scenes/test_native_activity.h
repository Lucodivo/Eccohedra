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

#include <dlfcn.h> // Android dynamic library utility functions

#include "noop_types.h"

#include "android_platform.h"
global_variable AAssetManager* assetManager_GLOBAL = nullptr;

#include "profiler/simplified_profiler.cpp"

#include "noop_math.h"
using namespace noop;

#include "asset_loader.h"
#include "texture_asset.h"
#include "cubemap_asset.h"
#include "model_asset.h"

#include "android_platform.cpp"
#include "shader_types_and_constants.h"
#include "vertex_attributes.h"
#include "file_locations.h"
#include "world_info.h"
#include "util.h"
#include "textures.h"
#include "shader_program.h"
#include "model.h"
#include "camera.h"
#include "gl_util.cpp"

#include "vertex_attributes.cpp"
#include "portal_scene.cpp"
