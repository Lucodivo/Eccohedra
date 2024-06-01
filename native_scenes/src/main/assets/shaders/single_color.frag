#version 310 es

precision lowp float;

layout (location = 0) in vec3 inNormal;

uniform vec3 baseColor;

struct InLight {
vec4 color;
vec4 pos;
};
layout (binding = 2, std140) uniform MultiLightInfoUBO {
  vec4 ambientLightColor;
  InLight dirPosLightStack[8];
  uint dirLightCount;
  uint posLightCount;
  uint padding1;
  uint padding2;
} lightInfoUbo;

layout (location = 0) out vec4 outColor;

void main() {
  InLight dirLight = lightInfoUbo.dirPosLightStack[0];
  float cosNormLight = dot(inNormal, dirLight.pos.xyz);
  vec3 dirColorContribution = (baseColor * dirLight.color.xyz) * (cosNormLight * dirLight.color.w);
  vec3 ambientColorContribution = (baseColor * lightInfoUbo.ambientLightColor.xyz) * lightInfoUbo.ambientLightColor.w;
  outColor = vec4(dirColorContribution + ambientColorContribution, 1.0);
}