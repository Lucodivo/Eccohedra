#version 300 es

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
	if(texWidth > texHeight) { // we want to use the smaller dimen as new width and height
		float oneOverWidth = 1.0 / texWidth;
		float texXStart = ((texWidth - texHeight) * 0.5) * oneOverWidth;
		float heightOverWidth = texHeight * oneOverWidth;
		croppedTexCoord = vec2(texXStart + (TextureCoord.x * heightOverWidth), TextureCoord.y);
	} else {
		float oneOverHeight = 1.0 / texHeight;
		float texYStart = ((texHeight - texWidth) * 0.5) * oneOverHeight;
		float widthOverHeight = texWidth * oneOverHeight;
		croppedTexCoord = vec2(TextureCoord.x, texYStart + (TextureCoord.y * widthOverHeight));
	}

	vec4 diffColor = texture(diffTexture, croppedTexCoord);

	FragColor = diffColor;
}