#version 320 es
layout (location = 0) in vec3 inPos;

layout (binding = 0, std140) uniform UBO { // base alignment   // aligned offset
  mat4 projection;                         // 64               // 0
  mat4 view;                               // 64               // 64
  mat4 model;                              // 64             // 128
} ubo;

void main()
{
  gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPos, 1.0);
}