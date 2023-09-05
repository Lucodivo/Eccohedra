#pragma once

// NOTE: "Zero is a reserved texture name and is never returned as a texture name by glGenTextures()"
// source: Naming A Texture Object in The Official Guide to Learning OpenGL, Version 1.1
#define TEXTURE_ID_NO_TEXTURE 0

// NOTE: Assuming 8 bits per stencil value
#define MAX_STENCIL_VALUE 0xFF

struct ShaderProgram {
  GLuint id;
  GLuint vertexShader;
  GLuint fragmentShader;
  GLuint noiseTextureId;
  const char* vertexFileName;
  const char* fragmentFileName;
  const char* noiseTextureFileName;
};

u32 projectionViewModelUBOBindingIndex = 0;
struct ProjectionViewModelUBO {  // base alignment   // aligned offset
  mat4 projection;               // 4                // 0
  mat4 view;                     // 4                // 64
  mat4 model;                    // 4                // 128
};

u32 fragUBOBindingIndex = 1;
struct FragUBO {
  f32 time;
};

u32 lightUBOBindingIndex = 2;
struct LightUniform {
  vec4 color; // NOTE: fourth component used for light power
  vec4 pos; // NOTE: fourth component for padding, currently un-defined
};
struct LightUBO {
  vec4 ambientLight;
  LightUniform dirPosLightStack[8];
  u32 dirLightCount;
  u32 posLightCount;
};

/*NOTE: GLSL Shader UBO Examples
layout (binding = 0, std140) uniform UBO {
  mat4 projection;
  mat4 view;
  mat4 model;
} ubo;

layout (binding = 1, std140) uniform FragUBO {
  float time;
} fragUbo;

layout (binding = 2, std140) uniform LightInfoUBO {
        vec3 directionalLightColor;
        vec3 ambientLightColor;
        vec3 directionalLightDirToSource;
} lightInfoUbo;
*/

const char* baseColorUniformName = "baseColor";
const char* skyboxTexUniformName = "skyboxTex";
const char* albedoTexUniformName = "albedoTex";
const char* normalTexUniformName = "normalTex";
const char* noiseTexUniformName = "noiseTex";
/* NOTE: GLSL Shader Texture Usage Examples
uniform vec4 baseColor;
uniform samplerCube skyboxTex;
uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform sampler2D noiseTex;
 */

const s32 skyboxActiveTextureIndex = 0;
const s32 albedoActiveTextureIndex = 1;
const s32 normalActiveTextureIndex = 2;
const s32 noiseActiveTextureIndex = 3;