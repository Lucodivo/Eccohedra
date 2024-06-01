#version 310 es

layout (location = 0) in vec3 inPos;

layout (binding = 0, std140) uniform UBO {
  mat4 projection;
  mat4 view;
//mat4 model; //NOTE: Available but not needed
} ubo;

layout (location = 0) out vec3 outTexCoord;

void main()
{
  outTexCoord = inPos;
  mat4 skyboxViewMat = mat4(mat3(ubo.view)); // remove translation
  vec4 pos = ubo.projection * skyboxViewMat * vec4(inPos, 1.0);
  // NOTE: z = w effectively sets depth to furthest away
  gl_Position = pos.xyww;
}  