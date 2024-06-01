#version 300 es
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoord;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

out vec3 Normal;
out vec3 FragPos;
out vec2 TextureCoord; 

void main()
{
  vec4 worldSpace = model * vec4(aPosition, 1.0);
  gl_Position = projection * view * worldSpace;
  FragPos = vec3(worldSpace);
  mat3 normalMat = mat3(transpose(inverse(model)));
  Normal = normalMat * aNormal;
  TextureCoord = aTextureCoord;
}