#version 320 es

precision highp float;

out vec4 FragColor;

#define MAX_STEPS 100
#define MISS_DIST 60.0
#define HIT_DIST 0.01

vec2 distanceRayToScene(vec3 rayOrigin, vec3 rayDir);
float sdCross(vec3 rayPos, vec3 dimen);
float sdRect(vec2 rayPos, vec2 dimen);
float sdMengerPrison(vec3 rayPos);

uniform vec2 viewPortResolution;
uniform vec3 rayOrigin;
uniform mat3 cameraRotationMat;
uniform int iterations;

const float boxDimen = 20.0;
const float halfBoxDimen = boxDimen * 0.5f;

void main()
{
    // Move (0,0) from bottom left to center
    vec2 pixelCoord = gl_FragCoord.xy-0.5*viewPortResolution.xy;
    // Scale y value to [-1.0, 1.0], scale x by same factor
    pixelCoord = pixelCoord / viewPortResolution.y;

    vec3 rayDir = vec3(pixelCoord.x, pixelCoord.y, 1.0);
    rayDir = normalize(cameraRotationMat * rayDir);

    vec2 dist = distanceRayToScene(rayOrigin, rayDir);

    if(dist.x > 0.0) { // hit
        vec3 col = vec3(1.0 - (dist.y / float(MAX_STEPS)));
        FragColor = vec4(col, 1.0);
    } else { // miss
        vec3 missColor = vec3(0.2, 0.2, 0.2);
        FragColor = vec4(missColor, 1.0);
    }
}

// returns vec2(dist, iterations)
// NOTE: ray dir is assumed to be normalized
vec2 distanceRayToScene(vec3 rayOrigin, vec3 rayDir) {

    float dist = 0.0;

    for(int i = 0; i < MAX_STEPS; i++) {
        vec3 pos = rayOrigin + (dist * rayDir);
        float posToScene = sdMengerPrison(pos);
        dist += posToScene;
        if(posToScene < HIT_DIST) return vec2(dist, i); // absolute value for posToScene incase the ray makes its way inside an object
        if(posToScene > MISS_DIST) break;
    }

    return vec2(-1.0f, MAX_STEPS);
}

float sdMengerPrison(vec3 rayPos) {
    vec3 prisonRay = mod(rayPos, boxDimen * 2.0f);
    prisonRay -= boxDimen; // move container origin to center
    float mengerPrisonDist = sdCross(prisonRay, vec3(halfBoxDimen));
    if(mengerPrisonDist > HIT_DIST) return mengerPrisonDist; // use dist of biggest crosses as bounding volume

    float scale = 1.0;
    for(int i = 0; i < iterations; ++i) {
        float boxedWorldDimen = boxDimen / scale;
        vec3 ray = mod(rayPos + boxedWorldDimen * 0.5, boxedWorldDimen);
        ray -= boxedWorldDimen * 0.5;
        ray *= scale;
        float crossesDist = sdCross(ray * 3.0, vec3(halfBoxDimen));
        scale *= 3.0;
        crossesDist /= scale;
        mengerPrisonDist = max(mengerPrisonDist, -crossesDist);
    }

    return mengerPrisonDist;
}

float sdRect(vec2 rayPos, vec2 dimen) {
    vec2 rayToCorner = abs(rayPos) - dimen;
    float maxDelta = min(max(rayToCorner.x, rayToCorner.y), 0.0);
    return length(max(rayToCorner, 0.0)) + maxDelta;
}

float sdCross(vec3 rayPos, vec3 dimen) {
    float distA = sdRect(rayPos.xy, dimen.xy);
    float distB = sdRect(rayPos.xz, dimen.xz);
    float distC = sdRect(rayPos.yz, dimen.yz);
    return min(distA,min(distB,distC));
}