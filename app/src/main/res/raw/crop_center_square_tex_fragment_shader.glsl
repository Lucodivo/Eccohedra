#version 300 es

// define float precision
precision highp float;

uniform sampler2D diffTexture;
uniform float texWidth;
uniform float texHeight;

in vec3 Normal;
in vec3 FragPos;
in vec2 TextureCoord;

out vec4 FragColor;

void main()
{
	vec2 croppedTexCoord;
	if(texWidth > texHeight) {
		float texXStart = ((texWidth - texHeight) / 2.0) / texWidth;
		float heightOverWidth = texHeight / texWidth;

		croppedTexCoord = vec2(texXStart + (TextureCoord.x * heightOverWidth), TextureCoord.y);
	} else {
		float texYStart = ((texHeight - texWidth) / 2.0) / texHeight;
		float widthOverHeight = texWidth / texHeight;

		croppedTexCoord = vec2(TextureCoord.x, texYStart + (TextureCoord.y * widthOverHeight));
	}

	vec4 diffColor = texture(diffTexture, croppedTexCoord);

	FragColor = diffColor;
}