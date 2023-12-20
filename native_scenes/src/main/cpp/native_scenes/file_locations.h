#pragma once

// ==== shaders ====
const char* posNormVertShaderFileLoc = "pos_norm.vert";

const char* skyboxVertexShaderFileLoc = "skybox.vert";
const char* skyboxFragmentShaderFileLoc = "skybox.frag";

const char* debugColorFragmentShaderFileLoc = "debug_color.frag";

const char* clearDepthFragmentShaderFileLoc = "clear_depth.frag";

// NOTE: Although we may prefer no stencil fragment shader,
// OpenGL ES Spec says a program may fail to link if
// "[the] program does not contain both a vertex shader and a fragment shader"
const char* posVertShaderFileLoc = "pos.vert";
const char* stencilFragmentShaderFileLoc = "stencil.frag";
