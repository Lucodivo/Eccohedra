#version 310 es

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout (binding = 0, std140) uniform UBO { // base alignment   // aligned offset
  mat4 projection;                         // 64               // 0
  mat4 view;                               // 64               // 64
  mat4 model;                              // 64               // 128
} ubo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outTexCoord;
layout (location = 2) out vec3 outFragmentWorldPos;
layout (location = 3) out vec3 outCameraWorldPos;

vec3 pullCameraPositionFromViewMat() {
  mat3 rotationTranspose = transpose(mat3(ubo.view));
  vec3 rotatedTranslation = ubo.view[3].xyz;
  vec3 originalTranslation = rotationTranspose * rotatedTranslation;
  return -(originalTranslation);
}

void main()
{
  mat3 normalMat = mat3(transpose(inverse(ubo.model))); // TODO: only necessary for non-uniform scaling
  vec4 worldPos = ubo.model * vec4(inPos, 1.0);

  outNormal = normalize(normalMat * inNormal);
  outTexCoord = inTexCoord;
  outFragmentWorldPos = worldPos.xyz;
  outCameraWorldPos = pullCameraPositionFromViewMat();
  gl_Position = ubo.projection * ubo.view * worldPos;
}