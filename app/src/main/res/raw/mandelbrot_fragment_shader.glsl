#version 300 es

precision highp float;

out vec4 FragColor;

#define MAX_ITERATIONS 100.0

uniform vec2 viewPortResolution;
uniform vec2 centerOffset;
uniform vec3 colorSub;
uniform float zoom;

void main() {
    // Move (0,0) from bottom left to center
    vec2 uv = gl_FragCoord.xy-0.5*viewPortResolution.xy;

    // Scale shortest dimension value to [-1.0, 1.0], scale other dimension by same factor
    float shortestDimension = (viewPortResolution.x > viewPortResolution.y) ? viewPortResolution.y : viewPortResolution.x;
    uv = uv / (shortestDimension * zoom);
    uv += centerOffset / shortestDimension;

    float iterations = 0.0;
    vec2 c = uv;
    while(iterations < MAX_ITERATIONS && length(uv) < 2.0) {
        // complex number squared
        uv = vec2((uv.x * uv.x) - (uv.y * uv.y), 2.0 * (uv.x * uv.y));
        // adding constant
        uv += c;
        iterations += 1.0;
    }

    float iterFraction = (iterations / MAX_ITERATIONS);
    vec3 color = vec3(1.0) - (colorSub * iterFraction);

    FragColor = vec4(color, 0.0);
}