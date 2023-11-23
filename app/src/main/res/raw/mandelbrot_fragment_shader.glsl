#version 320 es

precision highp float;

out vec4 FragColor;

uniform vec3 accentColor1;
uniform vec3 accentColor2;
uniform vec2 centerOffset;

uniform int maxIterations;
in vec2 pos;

vec4 colorFromIterations(int iterations);

vec2 complexSq(vec2 a_bi) {
    return vec2((a_bi.x * a_bi.x) - (a_bi.y * a_bi.y), 2.0 * (a_bi.x * a_bi.y));
}

float lengthSq(vec2 v) { return (v.x * v.x) + (v.y * v.y); }

void main() {
    int iterations = 0;
    vec2 z = vec2(0.0);
    vec2 c = pos + centerOffset;
    while(iterations < maxIterations && lengthSq(z) < 4.0) {
        // complex number squared
        z = complexSq(z);

        // adding constant
        z += c;
        iterations++;
    }

    FragColor = colorFromIterations(iterations);
}

vec4 colorFromIterations(int iterations) {
    const vec3 white = vec3(1.0, 1.0, 1.0);
    const vec3 black = vec3(0.0, 0.0, 0.0);
    vec3 darkestAccentCol1 = mix(accentColor1, black, .1);
    vec3 darkestAccentCol2 = mix(accentColor2, black, .1);
    vec3 lightestAccentCol1 = mix(accentColor1, white, .5);
    vec3 lightestAccentCol2 = mix(accentColor2, white, .5);

    const int numColors = 36;
    const int colorsPerSection = numColors / 6;
    const float mixPerIndex = 1.0 / float(colorsPerSection);

    int index = (iterations + (colorsPerSection - 2)) % numColors;
    int rangedIndex = index % colorsPerSection;

    vec3 color;
    if (index < colorsPerSection) { // [darkest1, accent1)
        color = mix(darkestAccentCol1, accentColor1, mixPerIndex * float(rangedIndex));
    } else if(index < (2 * colorsPerSection)) { // [accent1, lightest1)
        color = mix(accentColor1, lightestAccentCol1, mixPerIndex * float(rangedIndex));
    } else if(index < (3 * colorsPerSection)) { // [lightest1, lightest2)
        color = mix(lightestAccentCol1, lightestAccentCol2, mixPerIndex * float(rangedIndex));
    } else if(index < (4 * colorsPerSection)) { // [lightest2, accent2)
        color = mix(lightestAccentCol2, accentColor2, mixPerIndex * float(rangedIndex));
    } else if(index < 5 * colorsPerSection) { // [accent2, darkest2)
        color = mix(accentColor2, darkestAccentCol2, mixPerIndex * float(rangedIndex));
    } else { // [darkest2, darkest1)
        color = mix(darkestAccentCol2, darkestAccentCol1, mixPerIndex * float(rangedIndex));
    }

    float maxIterCheck = 1.0f - float(iterations / maxIterations); // Ensure max iterations is always black
    return vec4(color, 0.0) * maxIterCheck;
}