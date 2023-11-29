#pragma once

#include <math.h>

#include "noop_types.h"

// Note: If degrees/radians is not specified for angle, angle is assumed to be in radians

#define Min(x, y) (x < y ? x : y)
#define Max(x, y) (x > y ? x : y)

#define Pi32 3.14159265359f
#define PiOverTwo32 1.57079632679f
#define Tau32 6.28318530717958647692f
#define RadiansPerDegree (Pi32 / 180.0f)

#define cos30 0.86602540f
#define cos45 0.70710678f
#define cos60 0.5f

#define sin30 0.5f
#define sin45 0.70710678f
#define sin60 0.86602540f

#define COMPARISON_EPSILON 0.001f

namespace noop {
  struct vec2 {
    f32 values[2];
    inline f32 operator[](u32 i) const { return values[i]; }
    inline f32& operator[](u32 i) { return values[i]; }
  };

  struct vec3 {
    union {
      f32 values[3];
      vec2 xy;
    };
    inline f32 operator[](u32 i) const { return values[i]; }
    inline f32& operator[](u32 i) { return values[i]; }
  };

  struct vec4 {
    union {
      f32 values[4];
      vec3 xyz;
    };
    inline f32 operator[](u32 i) const { return values[i]; }
    inline f32& operator[](u32 i) { return values[i]; }
  };

// NOTE: column-major
  struct mat2 {
    f32 values[4];
    inline f32 operator[](u32 i) const { return values[i]; }
    inline f32& operator[](u32 i) { return values[i]; }
  };

// NOTE: column-major
  struct mat3 {
    f32 values[9];
    inline f32 operator[](u32 i) const { return values[i]; }
    inline f32& operator[](u32 i) { return values[i]; }
    inline mat2 toMat2(){ return mat2{
      values[0], values[1],
      values[3], values[4]
    };}
  };

// NOTE: column-major
  struct mat4 {
    f32 values[16];
    inline f32 operator[](u32 i) const { return values[i]; }
    inline f32& operator[](u32 i) { return values[i]; }
    inline mat3 toMat3(){ return mat3{
          values[0], values[1], values[2],
          values[4], values[5], values[6],
          values[8], values[9], values[10]
      };
    }
    inline static mat4 fromMat3(const mat3& M) {
      return mat4{
        M[0], M[1], M[2], 0.0f,
        M[3], M[4], M[5], 0.0f,
        M[6], M[7], M[8], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
      };
    }
  };

  struct quaternion {
    f32 r;
    union {
      struct {
        f32 i; f32 j; f32 k;
      };
      vec3 ijk;
    };
  };

  struct complex {
    f32 r;
    f32 i;
  };

  struct BoundingRect {
    vec2 min;
    vec2 diagonal;
  };

  struct BoundingBox {
    vec3 min;
    vec3 diagonal;
  };

// floating point
  b32 epsilonComparison(f32 a, f32 b, f32 epsilon = COMPARISON_EPSILON);

  b32 epsilonComparison(f64 a, f64 b, f64 epsilon = COMPARISON_EPSILON);

  f32 step(f32 edge, f32 x);

  f32 clamp(f32 minVal, f32 maxVal, f32 x);

  f32 smoothStep(f32 edge1, f32 edge2, f32 x);

  f32 lerp(f32 a, f32 b, f32 t);

  f32 sign(f32 x);

  f32 radians(f32 degrees);

// real-time rendering 4.7.2
// ex: screenWidth = 20.0f, screenDist = 30.0f will provide the horizontal field of view
// for a person sitting 30 units away from a 20 unit screen, assuming the screen is
// perpendicular to the line of sight.
// NOTE: Any units work as long as they are the same. Works for vertical and horizontal.
  f32 fieldOfView(f32 screenWidth, f32 screenDist);

// == vec2 ==
  b32 operator==(const vec2 &v1, const vec2 &v2);

  f32 dot(vec2 xy1, vec2 xy2);

  f32 magnitudeSquared(vec2 xy);

  f32 magnitude(vec2 xy);

  vec2 normalize(const vec2 &xy);

  vec2 normalize(f32 x, f32 y);

// v2 exists in the same half circle that centers around v1. Acute (<90º) angle between the two vectors.
  b32 similarDirection(vec2 v1, vec2 v2);

  vec2 operator-(const vec2 &xy);

  vec2 operator-(const vec2 &xy1, const vec2 &xy2);

  vec2 operator+(const vec2 &xy1, const vec2 &xy2);

  void operator-=(vec2 &xy1, const vec2 &xy2);

  void operator+=(vec2 &xy1, const vec2 &xy2);

  vec2 operator*(f32 s, vec2 xy);

  vec2 operator*(vec2 xy, f32 s);

  void operator*=(vec2 &xy, f32 s);

  vec2 operator/(const vec2 &xy, const f32 s);

  vec2 operator/(const vec2 &xy1, const vec2 &xy2);

  vec2 lerp(const vec2 &a, const vec2 &b, f32 t);

// == vec3 ==
  vec3 Vec3(vec2 xy, f32 z);

  vec3 Vec3(f32 value);

  b32 operator==(const vec3 &v1, const vec3 &v2);

  vec3 operator-(const vec3 &xyz);

  vec3 operator-(const vec3 &xyz1, const vec3 &xyz2);

  vec3 operator+(const vec3 &xyz1, const vec3 &xyz2);

  void operator-=(vec3 &xyz1, const vec3 &xyz2);

  void operator+=(vec3 &xyz1, const vec3 &xyz2);

  vec3 operator*(f32 s, vec3 xyz);

  vec3 operator*(vec3 xyz, f32 s);

  void operator*=(vec3 &xyz, f32 s);

  vec3 operator/(const vec3 &xyz, const f32 s);

  vec3 operator/(const vec3 &xyz1, const vec3 &xyz2);

  f32 dot(vec3 xyz1, vec3 xyz2);

  vec3 hadamard(const vec3 &xyz1, const vec3 &xyz2);

  vec3 cross(const vec3 &xyz1, const vec3 &xyz2);

  f32 magnitudeSquared(vec3 xyz);

  f32 magnitude(vec3 xyz);

  vec3 normalize(const vec3 &xyz);

  vec3 normalize(f32 x, f32 y, f32 z);

  b32 degenerate(const vec3 &v);

  vec3 projection(const vec3 &v1, const vec3 &ontoV2 /* assumed to be normalized */);

  vec3 perpendicularTo(const vec3 &v1, const vec3 &ontoV2 /* assumed to be normalized */);

// v2 exists in the same hemisphere that centers around v1. Acute (<90º) angle between the two vectors.
  b32 similarDirection(const vec3 &v1, const vec3 &v2);

  vec3 lerp(const vec3 &a, const vec3 &b, f32 t);

// vec4
  b32 operator==(const vec4 &v1, const vec4 &v2);

  vec4 Vec4(vec3 xyz, f32 w);

  vec4 Vec4(f32 x, vec3 yzw);

  vec4 Vec4(vec2 xy, vec2 zw);

  f32 dot(vec4 xyzw1, vec4 xyzw2);

  f32 magnitudeSquared(vec4 xyzw);

  f32 magnitude(vec4 xyzw);

  vec4 normalize(const vec4 &xyzw);

  vec4 normalize(f32 x, f32 y, f32 z, f32 w);

  vec4 operator-(const vec4 &xyzw);

  vec4 operator-(const vec4 &xyzw1, const vec4 &xyzw2);

  vec4 operator+(const vec4 &xyzw1, const vec4 &xyzw2);

  void operator-=(vec4 &xyzw1, const vec4 &xyzw2);

  void operator+=(vec4 &xyzw1, const vec4 &xyzw2);

  vec4 operator*(f32 s, vec4 xyzw);

  vec4 operator*(vec4 xyzw, f32 s);

  void operator*=(vec4 &xyzw, f32 s);

  vec4 operator/(const vec4 &xyzw, const f32 s);

  vec4 operator/(const vec4 &xyzw1, const vec4 &xyzw2);

  vec4 lerp(const vec4 &a, const vec4 &b, f32 t);

  vec4 min(const vec4 &xyzw1, const vec4 &xyzw2);

  vec4 max(const vec4 &xyzw1, const vec4 &xyzw2);

// Complex
// This angle represents a counter-clockwise rotation
  complex Complex(f32 angle);

  b32 operator==(const complex &c1, const complex &c2);

  f32 magnitudeSquared(complex c);

  f32 magnitude(complex c);

  vec2 operator*(const complex &ri, vec2 xy /* treated as complex number*/);

  void operator*=(vec2 &xy /* treated as complex number*/, const complex &ri);

// Quaternions
  b32 operator==(const quaternion &q1, const quaternion &q2);

  quaternion Quaternion(vec3 v, f32 angle);

  quaternion identity_quaternion();

  f32 magnitudeSquared(quaternion rijk);

  f32 magnitude(quaternion q);

// NOTE: Conjugate is equal to the inverse when the quaternion is unit length
  quaternion conjugate(quaternion q);

  f32 dot(const quaternion &q1, const quaternion &q2);

  quaternion normalize(const quaternion &q);

  vec3 rotate(const vec3 &v, const quaternion &q);

  quaternion operator*(const quaternion &q1, const quaternion &q2);

  quaternion operator+(const quaternion &q1, const quaternion &q2);

  quaternion operator-(const quaternion &q1, const quaternion &q2);

  quaternion operator*(const quaternion &q1, f32 s);

  quaternion operator*(f32 s, const quaternion &q1);

  quaternion operator/(const quaternion &q1, f32 s);

  quaternion lerp(quaternion a, quaternion b, f32 t);

  quaternion slerp(quaternion a, quaternion b, f32 t);

  quaternion orient(const vec3 &startOrientation, const vec3 &endOrientation);

// mat2
  b32 operator==(const mat2 &A, const mat2 &B);

  mat2 identity_mat2();

  mat2 scale_mat2(f32 scale);

  mat2 scale_mat2(vec3 scale);

  mat2 transpose(const mat2 &A);

  mat2 rotate(f32 radians);

// mat3
  b32 operator==(const mat3 &A, const mat3 &B);

  mat3 identity_mat3();

  mat3 scale_mat3(f32 scale);

  mat3 scale_mat3(vec3 scale);

  mat3 transpose(const mat3 &A);

  mat3 rotate_mat3(f32 radians, vec3 v);

  mat3 rotate_mat3(quaternion q);

  // vector treated as column vector
  vec3 operator*(const mat3 &M, const vec3 &v);

  // vector treated as row vector
  vec3 operator*(const vec3 &v, const mat3 &M);

  mat3 operator*(const mat3 &A, const mat3 &B);

// mat4
  mat4 Mat4(mat3 M);

  b32 operator==(const mat4 &A, const mat4 &B);

  mat4 identity_mat4();

  mat4 scale_mat4(f32 scale);

  mat4 scale_mat4(vec3 scale);

  mat4 translate_mat4(vec3 translation);

  mat4 transpose(const mat4 &A);

  mat4 rotate_xyPlane_mat4(f32 radians);

  mat4 rotate_mat4(f32 radians, vec3 v);

  mat4 scaleTrans_mat4(const f32 scale, const vec3 &translation);

  mat4 scaleTrans_mat4(const vec3 &scale, const vec3 &translation);

  mat4 scaleRotTrans_mat4(const vec3 &scale, const vec3 &rotAxis, const f32 angle,
                          const vec3 &translation);

  mat4 scaleRotTrans_mat4(const vec3 &scale, const quaternion &q, const vec3 &translation);

  mat4 rotate_mat4(quaternion q);

  vec4 operator*(const mat4 &M, const vec4 &v);

  mat4 operator*(const mat4 &A, const mat4 &B);

// real-time rendering 4.7.1
// This projection is for a canonical view volume goes from <-1,1>
  mat4 orthographic(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);

// real-time rendering 4.7.2
  mat4 perspective(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);

  mat4 perspectiveInverse(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);

// real-time rendering 4.7.2
// aspect ratio is equivalent to width / height
  mat4 perspective(f32 fovVert, f32 aspect, f32 n, f32 f);
  mat4 perspective_fovHorz(f32 fovHorz, f32 aspect, f32 n, f32 f);

  mat4 perspectiveInverse(f32 fovVert, f32 aspect, f32 n, f32 f);

/*
 * // NOTE: Oblique View Frustum Depth Projection and Clipping by Eric Lengyel (Terathon Software)
 * Arguments:
 * mat4 perspectiveMat: Perspective matrix that is being used for the rest of the scene
 * vec3 planeNormal_viewSpace: Plane normal in view space. MUST be normalized. This is a normal that points INTO the frustum, NOT one that is generally pointing towards the camera.
 * vec3 planePos_viewSpace: Any position on the plane in view space.
 */
  mat4 obliquePerspective(const mat4 &perspectiveMat, vec3 planeNormal_viewSpace,
                          vec3 planePos_viewSpace, f32 farPlane);
  mat4 obliquePerspective_fovHorz(f32 fovHorz, f32 aspect, f32 nearPlane, f32 farPlane, vec3 planeNormal_viewSpace, vec3 planePos_viewSpace);

/*
 * // NOTE: Oblique View Frustum Depth Projection and Clipping by Eric Lengyel (Terathon Software)
 * Arguments:
 * vec3 planeNormal_viewSpace: Plane normal in view space. MUST be normalized. This is a normal that points INTO the frustum, NOT one that is generally pointing towards the camera.
 * vec3 planePos_viewSpace: Any position on the plane in view space.
 */
  mat4 obliquePerspective(f32 fovVert, f32 aspect, f32 nearPlane, f32 farPlane,
                          vec3 planeNormal_viewSpace, vec3 planePos_viewSpace);

  void adjustAspectPerspProj(mat4 *projectionMatrix, f32 fovVert, f32 aspect);

  void adjustNearFarPerspProj(mat4 *projectionMatrix, f32 n, f32 f);

// etc
  bool insideRect(BoundingRect boundingRect, vec2 position);

  bool insideBox(BoundingBox boundingBox, vec3 position);

  bool overlap(BoundingRect bbA, BoundingRect bbB);

  bool overlap(BoundingBox bbA, BoundingBox bbB);
}

#undef COMPARISON_EPSILON