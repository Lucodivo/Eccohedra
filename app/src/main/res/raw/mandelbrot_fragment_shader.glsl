#version 320 es

precision highp float;

out vec4 FragColor;

uniform vec3 accentColors[16];
uniform vec2 centerOffset;

uniform int maxIterations;
in vec2 pos;

vec2 complexSq(vec2 a_bi) {
    return vec2((a_bi.x * a_bi.x) - (a_bi.y * a_bi.y), 2.0 * (a_bi.x * a_bi.y));
}

void main() {

    int iterations = 0;
    vec2 z = vec2(0.0);
    vec2 c = pos + centerOffset;
    while(iterations < maxIterations && length(z) < 2.0) {
        // complex number squared
        z = complexSq(z);

        // adding constant
        z += c;
        iterations++;
    }

    float maxIterCheck = 1.0f - float(iterations / maxIterations);
    int index1 = (iterations % 32) / 2;
    int index2 = ((iterations + 1) % 32) / 2;
    vec3 color = (accentColors[index1] + accentColors[index2]) / 2.0 * maxIterCheck;

    FragColor = vec4(color, 0.0);
}