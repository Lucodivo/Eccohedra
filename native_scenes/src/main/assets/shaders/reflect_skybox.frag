#version 310 es

precision highp float;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inCameraPos;

uniform samplerCube skyboxTex;

layout (location = 0) out vec4 outColor;

void main()
{
  vec3 cameraToPos = normalize(inPos - inCameraPos);
  vec3 reflected = reflect(cameraToPos, inNormal);
  // NOTE: samplerCubes assume y is up, so we must adjust accordingly
  vec3 reflectedYIsUp = vec3(reflected.x, reflected.z, -reflected.y);
  outColor = vec4(texture(skyboxTex, reflectedYIsUp).rgb, 1.0);
}