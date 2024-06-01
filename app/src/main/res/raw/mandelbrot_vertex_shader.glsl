#version 300 es

precision highp float;

layout (location = 0) in vec3 aPosition;

uniform vec2 viewPortResolution;
uniform mat2 rotationMat;
uniform float zoom;

out vec2 pos;

void main()
{
    vec2 uv;
    if (viewPortResolution.x > viewPortResolution.y) {
        uv = vec2(aPosition.x * (viewPortResolution.x / viewPortResolution.y), aPosition.y);
    } else {
        uv = vec2(aPosition.x, aPosition.y * (viewPortResolution.y / viewPortResolution.x));
    }
    uv = rotationMat * uv;
    uv *= 0.5;
    uv = uv / zoom;

    pos = uv;
    gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0);
}