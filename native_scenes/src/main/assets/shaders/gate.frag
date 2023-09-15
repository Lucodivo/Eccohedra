#version 320 es

precision highp float;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 inFragmentWorldPos;
layout (location = 3) in vec3 inCameraWorldPos;

layout (binding = 1, std140) uniform FragUBO {
  float time;
} fragUbo;

struct InLight {
  vec4 color;
  vec4 pos;
};

layout (binding = 2, std140) uniform LightInfoUBO {
  vec4 ambientLightColor;
  InLight dirPosLightStack[8];
  uint dirLightCount;
  uint posLightCount;
} lightInfoUbo;

uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform sampler2D noiseTex;

const float noiseStength = 40.0;

layout (location = 0) out vec4 outColor;

vec3 getNormal(vec2 texCoord);

void main() {
  vec2 time = vec2(fragUbo.time * 10.0);
  ivec2 albedoTexSize_i = textureSize(albedoTex, 0);
  ivec2 noiseTexSize_i = textureSize(noiseTex, 0);
  vec2 albedoTexSize = vec2(float(albedoTexSize_i.x), float(albedoTexSize_i.y));
  vec2 noiseTexSize = vec2(float(noiseTexSize_i.x), float(noiseTexSize_i.y));

  vec2 noiseTexCoord = ((inTexCoord * albedoTexSize) / noiseTexSize) + (vec2(-time.x, time.y) / noiseTexSize);
  float noise = texture(noiseTex, noiseTexCoord).r;
  noise = (noise - 0.5) * 2.0; // [-1,1]
  noise = noise * noiseStength;
  vec2 texCoordNoise = inTexCoord + (vec2(noise) / albedoTexSize);
  vec2 texCoordNoiseTime = texCoordNoise + (time / albedoTexSize);

  vec3 albedoColor = texture(albedoTex, texCoordNoiseTime).rgb;

  vec3 surfaceNormal = getNormal(texCoordNoiseTime);

  vec3 lightContribution = lightInfoUbo.ambientLightColor.rgb * lightInfoUbo.ambientLightColor.a;

  float directLightContribution = 0.0;
  for(uint i = 0u; i < lightInfoUbo.dirLightCount; i++) {
    vec3 surfaceToSource = lightInfoUbo.dirPosLightStack[i].pos.xyz;
    float cosTerm = max(dot(surfaceNormal, surfaceToSource), 0.0);
    lightContribution += lightInfoUbo.dirPosLightStack[i].color.rgb * lightInfoUbo.dirPosLightStack[i].color.a * cosTerm;
  }

  outColor = vec4(lightContribution * albedoColor, 1.0);
}

// Note: https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/pbr.vert
vec3 getNormal(vec2 texCoord)
{
  // Perturb normal, see http://www.thetenthplanet.de/archives/1180
  vec3 tangentNormal = texture(normalTex, texCoord).xyz * 2.0 - 1.0;

  vec3 q1 = dFdx(inFragmentWorldPos);
  vec3 q2 = dFdy(inFragmentWorldPos);
  vec2 st1 = dFdx(inTexCoord);
  vec2 st2 = dFdy(inTexCoord);

  vec3 N = normalize(inNormal);
  vec3 T = normalize(q1 * st2.t - q2 * st1.t);
  vec3 B = -normalize(cross(N, T));
  mat3 TBN = mat3(T, B, N);

  return normalize(TBN * tangentNormal);
}