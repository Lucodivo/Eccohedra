#version 300 es

uniform sampler2D diffTexture;

in vec3 Normal;
in vec3 FragPos;
in vec2 TextureCoord;

out vec4 FragColor;

void main()
{
	vec4 diffColor = texture(diffTexture, TextureCoord);
	if(diffColor.a < 0.1){
		discard;
	}
	FragColor = diffColor;
}