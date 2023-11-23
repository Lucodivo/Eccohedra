#pragma once

const vec3 WORLD_UP{0.0f, 0.0f, 1.0f};

const f32 MAX_MIN_PITCH_FIRST_PERSON= RadiansPerDegree * 85.0f;
const f32 MIN_PITCH_THIRD_PERSON = -25.0f * RadiansPerDegree;
const f32 MAX_PITCH_THIRD_PERSON = 65.0f * RadiansPerDegree;
const f32 DIST_FROM_PIVOT_THIRD_PERSON = 8.0f;

struct Camera {
  vec3 origin;
  vec3 up;
  vec3 right;
  vec3 forward;
  f32 pitch;
  f32 yaw;
  b32 thirdPerson;
};

// NOTE: pitch and yaw are set to radians
// NOTE: There is currently no support for lookAt_FirstPerson where forward should point directly up
void lookAt_FirstPerson(vec3 origin, vec3 focus, Camera* camera) {
  const f32 forwardDotUpThresholdMax = 0.996194f; // cos(5 degrees)
  const f32 forwardDotUpThresholdMin = -0.996194f; // cos(175 degrees)

  camera->thirdPerson = false;
  camera->origin = origin;
  camera->forward = normalize(focus - origin);
  f32 forwardDotUp = dot(camera->forward, WORLD_UP);
  if (forwardDotUp > forwardDotUpThresholdMax || forwardDotUp < forwardDotUpThresholdMin)
  {
    LOGE("Look At Camera Failed");
    camera->forward = normalize(camera->forward[0], camera->forward[1] + 0.01f, 0.0f);
  }

  camera->pitch = asin(camera->forward[2]);

  vec2 cameraForwardXYPlane = normalize(camera->forward[0], camera->forward[1]);
  camera->yaw = acos(cameraForwardXYPlane[0]);
  if(cameraForwardXYPlane[1] < 0) {
    camera->yaw = -camera->yaw;
  }

  camera->right = normalize(cross(camera->forward, WORLD_UP));
  camera->up = cross(camera->right, camera->forward);
}

/*
 * NOTE: Positive pitch offsets follows right hand rule (counter clockwise) with your thumb pointing in direction of X
 * NOTE: Positive yaw offsets follow right hand rule (counter clockwise) with your thumb pointing in direction of Z
 */
void updateCamera_FirstPerson(Camera* camera, vec3 posOffset, f32 pitchOffset, f32 yawOffset) {
  assert(!camera->thirdPerson);
  camera->origin += posOffset;

  camera->pitch += pitchOffset;
  if(camera->pitch > MAX_MIN_PITCH_FIRST_PERSON) {
    camera->pitch = MAX_MIN_PITCH_FIRST_PERSON;
  } else if(camera->pitch < -MAX_MIN_PITCH_FIRST_PERSON){
    camera->pitch = -MAX_MIN_PITCH_FIRST_PERSON;
  }

  camera->yaw += yawOffset;
  if(camera->yaw > Tau32) {
    camera->yaw -= Tau32;
  }

  // Calculate the new Front vector
  vec3 forward;
  f32 cosPitch = cos(camera->pitch);
  forward[0] = cos(camera->yaw) * cosPitch;
  forward[1] = sin(camera->yaw) * cosPitch;
  forward[2] = sin(camera->pitch);

  camera->forward = normalize(forward);
  // Also re-calculate the Right and Up vector
  camera->right = normalize(cross(camera->forward, WORLD_UP));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
  camera->up = normalize(cross(camera->right, camera->forward));
  camera->origin = camera->origin;
}

// NOTE: Yaw value of 0 degrees means we are looking in the direction of +x, 90=+y, 180=-x, 270=-y
// NOTE: Pitch value represents the angle between the vector from camera to pivot and the xy plane
void lookAt_ThirdPerson(vec3 pivot, vec3 forward, Camera* camera) {
  // Viewing angle measured between vector from pivot to camera and the xy plane
  const f32 startingPitch = 33.0f * RadiansPerDegree;

  vec2 xyForward = normalize(forward[0], forward[1]);
  vec2 xyPivotToCamera = -xyForward;

  camera->thirdPerson = true;
  camera->pitch = startingPitch;
  camera->yaw = acos(xyPivotToCamera[0]);
  if(xyPivotToCamera[1] < 0) {
    camera->yaw = -camera->yaw;
  }

  const f32 distBackFromPivot = DIST_FROM_PIVOT_THIRD_PERSON * cos(camera->pitch);
  const f32 distAboveFromPivot = DIST_FROM_PIVOT_THIRD_PERSON * sin(camera->pitch);

  camera->origin = pivot + Vec3(-distBackFromPivot * xyForward, distAboveFromPivot);
  camera->forward = normalize(pivot - camera->origin);

  camera->right = normalize(cross(camera->forward, WORLD_UP));
  camera->up = cross(camera->right, camera->forward);
}

void updateCamera_ThirdPerson(Camera* camera, vec3 pivotPoint, f32 pitchOffset, f32 yawOffset) {
  assert(camera->thirdPerson);

  camera->pitch += pitchOffset;
  if(camera->pitch > MAX_PITCH_THIRD_PERSON) {
    camera->pitch = MAX_PITCH_THIRD_PERSON;
  } else if(camera->pitch < MIN_PITCH_THIRD_PERSON){
    camera->pitch = MIN_PITCH_THIRD_PERSON;
  }

  camera->yaw += yawOffset;
  if(camera->yaw > Tau32) {
    camera->yaw -= Tau32;
  }

  const f32 distXYFromPivot = DIST_FROM_PIVOT_THIRD_PERSON * cos(camera->pitch);
  const f32 distAboveFromPivot = DIST_FROM_PIVOT_THIRD_PERSON * sin(camera->pitch);
  const f32 distXFromPivot = distXYFromPivot * cos(camera->yaw);
  const f32 distYFromPivot = distXYFromPivot * sin(camera->yaw);

  camera->origin = pivotPoint + vec3{distXFromPivot, distYFromPivot, distAboveFromPivot};
  camera->forward = normalize(pivotPoint - camera->origin);

  camera->right = normalize(cross(camera->forward, WORLD_UP));
  camera->up = cross(camera->right, camera->forward);
}

// NOTE: offsetPitch and offsetYaw in radians
mat4 getViewMat(const Camera& camera) {
  // Elements are accessed as mat[col][row] due to column-major layout
  mat4 translation{
                    1.0f,             0.0f,       0.0f,       0.0f,
                    0.0f,             1.0f,       0.0f,       0.0f,
                    0.0f,             0.0f,       1.0f,       0.0f,
        -camera.origin[0], -camera.origin[1], -camera.origin[2], 1.0f
  };

  // The camera matrix "measures" the world against it's axes
  // OpenGL clips down the negative z-axis so we negate our forward to effectively cancel out that negation
  mat4 measure{
          camera.right[0], camera.up[0], -camera.forward[0], 0.0f,
          camera.right[1], camera.up[1], -camera.forward[1], 0.0f,
          camera.right[2], camera.up[2], -camera.forward[2], 0.0f,
                    0.0f,        0.0f,              0.0f, 1.0f
  };

  // Return lookAt_FirstPerson matrix as combination of translation and measure matrix
  // Since matrices should be read right to left we want to...
  //    - First center the camera at the origin by translating itself and the entire world
  //    - Then we measure how the world lines up with the camera at the origin
  // All rotation vectors in this instance are orthonormal, so no stretching/squashing occurs is present in the
  // resulting matrix. All angles and area preserved.
  mat4 resultMat = measure * translation;
  return resultMat; // Remember to read from right to left (first translation then measure)
}