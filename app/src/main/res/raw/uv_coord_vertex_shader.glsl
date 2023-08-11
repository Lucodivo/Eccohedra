#version 320 es
layout (location = 0) in vec3 aPosition;

void main()
{
    gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0);
}
