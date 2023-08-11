#version 320 es

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;

layout (binding = 0, std140) uniform UBO { // base alignment   // aligned offset
  mat4 projection;                         // 64               // 0
  mat4 view;                               // 64               // 64
  mat4 model;                              // 64             // 128
} ubo;

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outCameraPos;

vec3 pullCameraPositionFromViewMat() {
  mat3 rotationTranspose = transpose(mat3(ubo.view));
  vec3 rotatedTranslation = ubo.view[3].xyz;
  vec3 originalTranslation = rotationTranspose * rotatedTranslation;
  return -(originalTranslation);
}

void main()
{
  mat3 normalMat = mat3(transpose(inverse(ubo.model))); // TODO: only necessary for non-uniform scaling
  outNormal = normalize(normalMat * inNormal);
  outCameraPos = pullCameraPositionFromViewMat();
  outPos = vec3(ubo.model * vec4(inPos, 1.0));
  gl_Position = ubo.projection * ubo.view * vec4(outPos, 1.0f);
}