#version 320 es

precision highp float;

out vec4 FragColor;

uniform vec3 accentColor;
uniform vec2 centerOffset;

in vec2 pos;

const int maxIterations = 1000;
const vec3 baseColor = vec3(1.0, 1.0, 1.0);

void main() {
    int iterations = 0;
    vec2 z = vec2(0.0);
    vec2 c = pos + centerOffset;
    while(iterations < maxIterations) {
        float xSq = z.x * z.x;
        float ySq = z.y * z.y;
        if(xSq + ySq > 4.0) { break; }

        // complex number squared
        z = vec2(xSq - ySq, 2.0 * (z.x * z.y));

        // adding constant
        z += c;
        iterations++;
    }

    float iterOverMax = float(iterations) / float(maxIterations);
    vec3 outsideSetColor = mix(baseColor, accentColor, iterOverMax);
    float mandelbrotSetNullifier = 1.0 - floor(iterOverMax);
    FragColor = vec4(outsideSetColor * mandelbrotSetNullifier, 1.0);
}