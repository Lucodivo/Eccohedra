#version 300 es

// define float precision
precision highp float;

out vec4 FragColor;

#define MAX_STEPS 150
#define MISS_DIST 100.0
#define HIT_DIST 0.01

const vec4 sphere = vec4(1.5, 1.5, 1.5, 0.5);
const float floorDistance = 0.0;
const float lightOrbitRadius = 60.0f;
const float lightOrbitSpeed = 1.0f / 64.0f;

float distPosToScene(vec3 pos);
float distanceRayToScene(vec3 rayOrigin, vec3 rayDir);
float getLight(vec3 surfacePos);
vec3 getNormal(vec3 surfacePos);

uniform vec2 viewPortResolution;
uniform vec3 rayOrigin;
uniform float elapsedTime;
uniform mat4 viewRotationMat;

float sinElapsedTime;
float cosElapsedTime;

void main()
{
    sinElapsedTime = sin(elapsedTime * lightOrbitSpeed);
    cosElapsedTime = cos(elapsedTime * lightOrbitSpeed);

    vec2 uv = (gl_FragCoord.xy-0.5*viewPortResolution.xy)/viewPortResolution.y;

    vec3 rayDir = vec3(uv.x, uv.y, 1.0); // NOTE: Expected to be normalized!
    rayDir = vec3(vec4(rayDir, 0.0) * viewRotationMat);
    rayDir = normalize(rayDir);

    float dist = distanceRayToScene(rayOrigin, rayDir);
    vec3 worldColor = vec3((sinElapsedTime + 1.0) / 2.0, cos(elapsedTime / 77.0), (cosElapsedTime + 1.0) / 2.0);
    //vec3 worldColor = vec3(1.0, 1.0, 0.4);

    if(dist > 0.0) { // hit
        vec3 surfacePos = rayOrigin + (rayDir * dist);
        float diffuse = getLight(surfacePos);
        vec3 col = vec3(diffuse);

        //dist = dist / 7.0;
        //vec3 col = vec3(dist);

        FragColor = vec4(col * (worldColor * 0.75), 1.0);
    } else { // miss

        FragColor = vec4(worldColor, 1.0);
    }
}

vec3 getNormal(vec3 surfacePos) {
    float dist = distPosToScene(surfacePos);
    vec2 epsilon = vec2(0.1, 0);
    vec3 normal = vec3(dist) - vec3(
    distPosToScene(surfacePos - epsilon.xyy),
    distPosToScene(surfacePos - epsilon.yxy),
    distPosToScene(surfacePos - epsilon.yyx)
    );
    return normalize(normal);
}

float getLight(vec3 surfacePos) {
    vec3 lightPos = vec3(0, 40.0, 0.0);
    lightPos.xz += vec2(sinElapsedTime * lightOrbitRadius, cosElapsedTime * lightOrbitRadius);
    vec3 lightDir = normalize(lightPos - surfacePos);
    vec3 normal = getNormal(surfacePos);

    float diff = clamp(dot(normal, lightDir), 0.0, 1.0);

    // calculate for shadows
    //float dist = distanceRayToScene(surfacePos + (normal * HIT_DIST * 2.0), lightDir);
    //if(dist < length(lightPos - surfacePos)) diff *= 0.1;

    return diff;
}

// NOTE: ray dir is assumed to be normalized
float distanceRayToScene(vec3 rayOrigin, vec3 rayDir) {

    float dist = 0.0;

    for(int i = 0; i < MAX_STEPS; i++) {

        vec3 pos = rayOrigin + (dist * rayDir);
        float posToScene = distPosToScene(pos);
        dist += posToScene;
        if(posToScene < HIT_DIST) return dist;
        if(posToScene > MISS_DIST) break;
    }

    return -1.0f;
}

float distPosToScene(vec3 pos) {
    //float distToSphere = distance(sphere.xyz, pos.xyz) - sphere.w;
    //float distToPlane = pos.y;
    //return min(distToSphere, distToPlane);
    return distance(mod(pos.xyz, 3.0), sphere.xyz) - sphere.w;
}