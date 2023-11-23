#version 320 es

precision lowp float;

uniform vec3 baseColor;

layout (location = 0) out vec4 outColor;

void main()
{
  outColor = vec4(baseColor, 1.0);
}