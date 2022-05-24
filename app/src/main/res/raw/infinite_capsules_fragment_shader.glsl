#version 300 es

// define float precision
precision highp float;

out vec4 FragColor;

#define MAX_STEPS 100
#define MISS_DIST 50.0
#define HIT_DIST 0.01

const float lightRadius = 0.5f;

float totalDistToScene(vec3 rayOrigin, vec3 rayDir);
float distToScene(vec3 rayPos);
float distToCapsule(vec3 rayPos, vec3 posA, vec3 posB, float radius);
float distToSphere(vec3 rayPos, vec3 lightPos, float radius);
float getLight(vec3 surfacePos);
vec3 getNormal(vec3 surfacePos);

uniform vec2 viewPortResolution;
uniform vec3 rayOrigin;
uniform float elapsedTime;
uniform mat3 cameraRotationMat;
uniform vec3 lightPos;
uniform vec3 lightColor;

const vec3 missColor = vec3(0.2, 0.2, 0.2);

const float capsuleLineWidth = 3.0f;
const float capsuleLineHalfWidth = capsuleLineWidth * 0.5f;
const float oneOverCapsuleLineWidth = 1.0f / capsuleLineWidth;
vec3 capsuleCenterPosA = vec3(-capsuleLineHalfWidth, 0.0, 0.0);
vec3 capsuleCenterPosB = vec3(capsuleLineHalfWidth, 0.0, 0.0);
const vec3 capsuleContainerDimens = vec3(6.0f, 6.0f, 6.0f);
const vec3 offset = capsuleContainerDimens * 0.5f; // this value also

void main()
{
    vec2 uv = (gl_FragCoord.xy - (0.5 * viewPortResolution.xy))/viewPortResolution.y;

    vec3 rayDir = vec3(uv.x, uv.y, 1.0); // NOTE: Expected to be normalized!
    rayDir = normalize(cameraRotationMat * rayDir);

    // move capsule centers
    capsuleCenterPosA = cameraRotationMat * capsuleCenterPosA;
    capsuleCenterPosB = cameraRotationMat * capsuleCenterPosB;
    capsuleCenterPosA = capsuleCenterPosA + offset;
    capsuleCenterPosB = capsuleCenterPosB + offset;

    float dist = totalDistToScene(rayOrigin, rayDir);

    if(dist > 0.0) { // hit
        vec3 surfacePos = rayOrigin + (rayDir * dist);
        float diffuse = getLight(surfacePos);
        vec3 col = vec3(diffuse);

        FragColor = vec4(col * lightColor, 1.0);
    } else { // miss
        FragColor = vec4(missColor, 1.0);
    }
}

const vec2 normalEpsilon = vec2(0.001, 0.0);
vec3 getNormal(vec3 surfacePos) {
    float dist = distToScene(surfacePos);
    float xDist = distToScene(surfacePos + normalEpsilon.xyy);
    float yDist = distToScene(surfacePos + normalEpsilon.yxy);
    float zDist = distToScene(surfacePos + normalEpsilon.yyx);
    vec3 normal = (vec3(xDist, yDist, zDist) - dist) / normalEpsilon.x;
    return normalize(normal);
}


float getLight(vec3 surfacePos) {
    vec3 lightDir = normalize(lightPos - surfacePos);
    vec3 normal = getNormal(surfacePos);
    float lightNormalSimilarDir = clamp(dot(normal, lightDir), 0.0, 1.0);
    return lightNormalSimilarDir;
}

// NOTE: ray dir is assumed to be normalized
float totalDistToScene(vec3 rayOrigin, vec3 rayDir) {
    float dist = 0.0;
    for(int i = 0; i < MAX_STEPS; i++) {
        vec3 pos = rayOrigin + (dist * rayDir);
        float posToScene = distToScene(pos);
        dist += posToScene;
        if(abs(posToScene) < HIT_DIST) return dist;
        if(posToScene > MISS_DIST) break;
    }
    return -1.0f;
}


float distToScene(vec3 rayPos) {
    vec3 rayPosCapsuleContainer = mod(rayPos, capsuleContainerDimens);
    float infiniteCapsuleDistance = distToCapsule(rayPosCapsuleContainer, capsuleCenterPosA, capsuleCenterPosB, 1.0f);
    float lightSphereDistance = distToSphere(rayPos, lightPos, lightRadius);
    return min(infiniteCapsuleDistance, lightSphereDistance);
}

float distToSphere(vec3 rayPos, vec3 lightPos, float radius) {
    return (distance(rayPos, lightPos) - radius);
}

float distToCapsule(vec3 rayPos, vec3 posA, vec3 posB, float radius) {
    // Line segments from point A to B (line segment of capsule)
    vec3 aToB = posB - posA;
    // Line from point A to ray's position
    vec3 aToRayPos = rayPos - posA;

    // find the projection of the line from A to the ray's position onto the capsule's line segment
    float abCosTheta = dot(aToB, aToRayPos);
    // float magnitudeAToB = capsuleLineWidth;
    float projectionAToRayOnAToB = abCosTheta * oneOverCapsuleLineWidth; // = abCosTheta / magnitudeAToB

    // Use the projection to walk down the capsule's line segment and find the closest point
    vec3 closestPoint = posA + (clamp(projectionAToRayOnAToB * oneOverCapsuleLineWidth, 0.0, 1.0) * aToB);
    return length(rayPos - closestPoint) - radius;
}