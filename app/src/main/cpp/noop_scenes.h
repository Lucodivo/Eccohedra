#pragma once

#define GLFW_INCLUDE_NONE // ensure GLFW doesn't load OpenGL headers
#include <GLFW/glfw3.h>
#undef APIENTRY // collides with minwindef.h
#include <glad/glad.h>
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

// platform/input
#include <windows.h>
#include <Xinput.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "nlohmann/json.hpp"

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_JSON
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tinygltf/tiny_gltf.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "ImGuiFileDialog/ImGuiFileDialog.h"

#undef far
#undef near

#include "noop_types.h"
#include "noop_math.h"
#include "shader_types_and_constants.h"
#include "cstring_ring_buffer.h"
#include "vertex_attributes.h"
#include "file_locations.h"
#include "save_file.h"
#include "input.h"
#include "util.h"
#include "textures.h"
#include "shader_program.h"
#include "model.h"
#include "camera.h"

#include "glfw_util.cpp"
#include "input.cpp"
#include "vertex_attributes.cpp"
#include "portal_scene.cpp"
