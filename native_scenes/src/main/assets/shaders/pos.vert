#version 320 es
layout (location = 0) in vec3 inPos;

layout (binding = 0, std140) uniform UBO {
  mat4 projection;
  mat4 view;
  mat4 model;
} ubo;

void main() {
  gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPos, 1.0);
}