#include "noop_math.h"

// TODO: No optimizations have been made in this file. Ideas: intrinsics, sse, better usage of temporary memory.

namespace noop {

// floating point
  b32 epsilonComparison(f32 a, f32 b, f32 epsilon) {
    f32 diff = a - b;
    return (diff <= epsilon && diff >= -epsilon);
  }

  b32 epsilonComparison(f64 a, f64 b, f64 epsilon) {
    f64 diff = a - b;
    return (diff <= epsilon && diff >= -epsilon);
  }

  f32 step(f32 edge, f32 x) {
    return x < edge ? 0.0f : 1.0f;
  }

  f32 clamp(f32 minVal, f32 maxVal, f32 x) {
    return Min(maxVal, Max(minVal, x));
  }

  f32 smoothStep(f32 edge1, f32 edge2, f32 x) {
    f32 t = clamp((x - edge1) / (edge2 - edge1), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
  }

  f32 lerp(f32 a, f32 b, f32 t) {
    assert(t >= 0.0f && t <= 1.0f);
    return a - ((a + b) * t);
  }

  f32 sign(f32 x) {
    if(x > 0.0f) return (1.0f);
    if(x < 0.0f) return (-1.0f);
    return (0.0f);
  }

  f32 radians(f32 degrees) {
    return RadiansPerDegree * degrees;
  }

// vec2
  b32 operator==(const vec2& v1, const vec2& v2) {
    return epsilonComparison(v1[0], v2[0]) &&
           epsilonComparison(v1[1], v2[1]);
  }

  f32 dot(vec2 xy1, vec2 xy2) {
    return (xy1[0] * xy2[0]) + (xy1[1] * xy2[1]);
  }

  f32 magnitudeSquared(vec2 xy) {
    return (xy[0] * xy[0]) + (xy[1] * xy[1]);
  }

  f32 magnitude(vec2 xy) {
    return sqrtf((xy[0] * xy[0]) + (xy[1] * xy[1]));
  }

  vec2 normalize(const vec2& xy) {
    f32 mag = magnitude(xy);
    assert(mag != 0.0f);
    f32 magInv = 1.0f / mag;
    return vec2{xy[0] * magInv, xy[1] * magInv};
  }

  vec2 normalize(f32 x, f32 y) {
    f32 mag = sqrtf(x * x + y * y);
    assert(mag != 0.0f);
    f32 magInv = 1.0f / mag;
    return vec2{x * magInv, y * magInv};
  }

// less than a 90 degree angle between the two vectors
// v2 exists in the same half circle that centers around v1
  b32 similarDirection(vec2 v1, vec2 v2) {
    return dot(v1, v2) > 0.0f;
  }

  vec2 operator-(const vec2& xy) {
    return vec2{-xy[0], -xy[1]};
  }

  vec2 operator-(const vec2& xy1, const vec2& xy2) {
    return vec2{xy1[0] - xy2[0], xy1[1] - xy2[1]};
  }

  vec2 operator+(const vec2& xy1, const vec2& xy2) {
    return vec2{xy1[0] + xy2[0], xy1[1] + xy2[1]};
  }

  void operator-=(vec2& xy1, const vec2& xy2) {
    xy1[0] -= xy2[0];
    xy1[1] -= xy2[1];
  }

  void operator+=(vec2& xy1, const vec2& xy2) {
    xy1[0] += xy2[0];
    xy1[1] += xy2[1];
  }

  vec2 operator*(f32 s, vec2 xy) {
    return vec2{xy[0] * s, xy[1] * s};
  }

  vec2 operator*(vec2 xy, f32 s) {
    return vec2{xy[0] * s, xy[1] * s};
  }

  void operator*=(vec2& xy, f32 s) {
    xy[0] *= s;
    xy[1] *= s;
  }

  vec2 operator/(const vec2& xy, const f32 s) {
    assert(s != 0);
    f32 scaleInv = 1.0f / s;
    return {xy[0] * scaleInv, xy[1] * scaleInv};
  }

  vec2 operator/(const vec2& xy1, const vec2& xy2) {
    assert(xy2[0] != 0 && xy2[1] != 0);
    return {xy1[0] / xy2[0], xy1[1] / xy2[1]};
  }

  vec2 lerp(const vec2& a, const vec2& b, f32 t) {
    assert(t >= 0.0f && t <= 1.0f);
    return a - ((a + b) * t);
  }

  bool lineSegmentsIntersection(vec2 a1, vec2 a2, vec2 b1, vec2 b2, vec2* intersection) {
    // From Andre Lamothe's Tricks of the Windows Game Programming Gurus
    vec2 a1toa2 = a2 - a1;
    vec2 b1tob2 = b2 - b1;

    float s, t;
    s = (-a1toa2[1] * (a1[0] - b1[0]) + a1toa2[0] * (a1[1] - b1[1])) / (-b1tob2[0] * a1toa2[1] + a1toa2[0] * b1tob2[1]);
    t = ( b1tob2[0] * (a1[1] - b1[1]) - b1tob2[1] * (a1[0] - b1[0])) / (-b1tob2[0] * a1toa2[1] + a1toa2[0] * b1tob2[1]);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
      // Collision detected
      if(intersection != nullptr) {
        intersection->values[0] = a1[0] + (t * a1toa2[0]);
        intersection->values[1] = a1[1] + (t * a1toa2[1]);
      }
      return true;
    }

    return false; // No collision
  }

  f32 determinant(mat2 m) {
    // | a b |
    // | c d | = ad - bc
    return m[0]*m[3] - m[2]*m[1];
  }

// vec3
  vec3 Vec3(vec2 xy, f32 z) {
    return vec3{xy[0], xy[1], z};
  }

  vec3 Vec3(f32 value) {
    return vec3{value, value, value};
  }

  b32 operator==(const vec3& v1, const vec3& v2) {
    return epsilonComparison(v1[0], v2[0]) &&
           epsilonComparison(v1[1], v2[1]) &&
           epsilonComparison(v1[2], v2[2]);
  }

  vec3 operator-(const vec3& xyz) {
    return vec3{-xyz[0], -xyz[1], -xyz[2]};
  }

  vec3 operator-(const vec3& xyz1, const vec3& xyz2) {
    return vec3{xyz1[0] - xyz2[0], xyz1[1] - xyz2[1], xyz1[2] - xyz2[2]};
  }

  vec3 operator+(const vec3& xyz1, const vec3& xyz2) {
    return vec3{xyz1[0] + xyz2[0], xyz1[1] + xyz2[1], xyz1[2] + xyz2[2]};
  }

  void operator-=(vec3& xyz1, const vec3& xyz2) {
    xyz1[0] -= xyz2[0];
    xyz1[1] -= xyz2[1];
    xyz1[2] -= xyz2[2];
  }

  void operator+=(vec3& xyz1, const vec3& xyz2) {
    xyz1[0] += xyz2[0];
    xyz1[1] += xyz2[1];
    xyz1[2] += xyz2[2];
  }

  vec3 operator*(f32 s, vec3 xyz) {
    return vec3{xyz[0] * s, xyz[1] * s, xyz[2] * s};
  }

  vec3 operator*(vec3 xyz, f32 s) {
    return vec3{xyz[0] * s, xyz[1] * s, xyz[2] * s};
  }

  void operator*=(vec3& xyz, f32 s) {
    xyz[0] *= s;
    xyz[1] *= s;
    xyz[2] *= s;
  }

  vec3 operator/(const vec3& xyz, const f32 s) {
    assert(s != 0);
    f32 scaleInv = 1.0f / s;
    return {xyz[0] * scaleInv, xyz[1] * scaleInv, xyz[2] * scaleInv};
  }

  vec3 operator/(const vec3& xyz1, const vec3& xyz2) {
    assert(xyz2[0] != 0 && xyz2[1] != 0 && xyz2[2] != 0);
    return {xyz1[0] / xyz2[0], xyz1[1] / xyz2[1], xyz1[2] / xyz2[2]};
  }

  f32 dot(vec3 xyz1, vec3 xyz2) {
    return (xyz1[0] * xyz2[0]) + (xyz1[1] * xyz2[1]) + (xyz1[2] * xyz2[2]);
  }

  vec3 hadamard(const vec3& xyz1, const vec3& xyz2) {
    return vec3{xyz1[0] * xyz2[0], xyz1[1] * xyz2[1], xyz1[2] * xyz2[2]};
  }

  vec3 cross(const vec3& xyz1, const vec3& xyz2) {
    return vec3{(xyz1[1] * xyz2[2] - xyz2[1] * xyz1[2]),
                (xyz1[2] * xyz2[0] - xyz2[2] * xyz1[0]),
                (xyz1[0] * xyz2[1] - xyz2[0] * xyz1[1])};
  }

  f32 magnitudeSquared(vec3 xyz) {
    return (xyz[0] * xyz[0]) + (xyz[1] * xyz[1]) + (xyz[2] * xyz[2]);
  }

  f32 magnitude(vec3 xyz) {
    return sqrtf((xyz[0] * xyz[0]) + (xyz[1] * xyz[1]) + (xyz[2] * xyz[2]));
  }

  vec3 normalize(const vec3& xyz) {
    f32 mag = magnitude(xyz);
    assert(mag != 0.0f);
    f32 magInv = 1.0f / mag;
    return magInv * xyz;
  }

  vec3 normalize(f32 x, f32 y, f32 z) {
    f32 mag = sqrtf(x * x + y * y + z * z);
    assert(mag != 0.0f);
    f32 magInv = 1.0f / mag;
    return vec3{x * magInv, y * magInv, z * magInv};
  }

  b32 degenerate(const vec3& v) {
    return v == vec3{0.0f, 0.0f, 0.0f};
  }

// v2 is assumed to be normalized
  vec3 projection(const vec3& v1, const vec3& ontoV2) {
    f32 dotV1V2 = dot(v1, ontoV2);
    return dotV1V2 * ontoV2;
  }

// v2 is assumed to be normalized
  vec3 perpendicularTo(const vec3& v1, const vec3& ontoV2) {
    vec3 parallel = projection(v1, ontoV2);
    return v1 - parallel;
  }

// less than a 90 degree angle between the two vectors
// v2 exists in the same hemisphere that centers around v1
  b32 similarDirection(const vec3& v1, const vec3& v2) {
    return dot(v1, v2) > 0.0f;
  }

  vec3 lerp(const vec3& a, const vec3& b, f32 t) {
    assert(t >= 0.0f && t <= 1.0f);
    return a - ((a + b) * t);
  }

// vec4
  vec4 Vec4(vec2 xy, f32 z,  f32 w) {
    return vec4{xy[0], xy[1], z, w};
  }

  vec4 Vec4(vec3 xyz, f32 w) {
    return vec4{xyz[0], xyz[1], xyz[2], w};
  }

  b32 operator==(const vec4& v1, const vec4& v2) {
    return epsilonComparison(v1[0], v2[0]) &&
           epsilonComparison(v1[1], v2[1]) &&
           epsilonComparison(v1[2], v2[2]) &&
           epsilonComparison(v1[3], v2[3]);
  }

  f32 dot(vec4 xyzw1, vec4 xyzw2) {
    return (xyzw1[0] * xyzw2[0]) + (xyzw1[1] * xyzw2[1]) + (xyzw1[2] * xyzw2[2]) + (xyzw1[3] * xyzw2[3]);
  }

  f32 magnitudeSquared(vec4 xyzw) {
    return (xyzw[0] * xyzw[0]) + (xyzw[1] * xyzw[1]) + (xyzw[2] * xyzw[2]) + (xyzw[3] * xyzw[3]);
  }

  f32 magnitude(vec4 xyzw) {
    return sqrtf((xyzw[0] * xyzw[0]) + (xyzw[1] * xyzw[1]) + (xyzw[2] * xyzw[2]) + (xyzw[3] * xyzw[3]));
  }

  vec4 normalize(const vec4& xyzw) {
    f32 mag = magnitude(xyzw);
    assert(mag != 0.0f);
    f32 magInv = 1.0f / mag;
    return vec4{xyzw[0] * magInv, xyzw[1] * magInv, xyzw[2] * magInv, xyzw[3] * magInv};
  }

  vec4 normalize(f32 x, f32 y, f32 z, f32 w) {
    f32 mag = sqrtf(x * x + y * y + z * z + w * w);
    assert(mag != 0.0f);
    f32 magInv = 1.0f / mag;
    return vec4{x * magInv, y * magInv, z * magInv, w * magInv};
  }

  vec4 operator-(const vec4& xyzw) {
    return vec4{-xyzw[0], -xyzw[1], -xyzw[2], -xyzw[3]};
  }

  vec4 operator-(const vec4& xyzw1, const vec4& xyzw2) {
    return vec4{xyzw1[0] - xyzw2[0], xyzw1[1] - xyzw2[1], xyzw1[2] - xyzw2[2], xyzw1[3] - xyzw2[3]};
  }

  vec4 operator+(const vec4& xyzw1, const vec4& xyzw2) {
    return vec4{xyzw1[0] + xyzw2[0], xyzw1[1] + xyzw2[1], xyzw1[2] + xyzw2[2], xyzw1[3] + xyzw2[3]};
  }

  void operator-=(vec4& xyzw1, const vec4& xyzw2) {
    xyzw1[0] -= xyzw2[0];
    xyzw1[1] -= xyzw2[1];
    xyzw1[2] -= xyzw2[2];
    xyzw1[3] -= xyzw2[3];
  }

  void operator+=(vec4& xyzw1, const vec4& xyzw2) {
    xyzw1[0] += xyzw2[0];
    xyzw1[1] += xyzw2[1];
    xyzw1[2] += xyzw2[2];
    xyzw1[3] += xyzw2[3];
  }

  vec4 operator*(f32 s, vec4 xyzw) {
    return vec4{xyzw[0] * s, xyzw[1] * s, xyzw[2] * s, xyzw[3] * s};
  }

  vec4 operator*(vec4 xyzw, f32 s) {
    return vec4{xyzw[0] * s, xyzw[1] * s, xyzw[2] * s, xyzw[3] * s};
  }

  void operator*=(vec4& xyzw, f32 s) {
    xyzw[0] *= s;
    xyzw[1] *= s;
    xyzw[2] *= s;
    xyzw[3] *= s;
  }

  vec4 operator/(const vec4& xyzw, const f32 s) {
    assert(s != 0);
    f32 scaleInv = 1.0f / s;
    return {xyzw[0] * scaleInv, xyzw[1] * scaleInv, xyzw[2] * scaleInv, xyzw[3] * scaleInv};
  }

  vec4 operator/(const vec4& xyzw1, const vec4& xyzw2) {
    assert(xyzw2[0] != 0 && xyzw2[1] != 0 && xyzw2[2] != 0 && xyzw2[3] != 0);
    return {xyzw1[0] / xyzw2[0], xyzw1[1] / xyzw2[1], xyzw1[2] / xyzw2[2], xyzw1[3] / xyzw2[3]};
  }

  vec4 lerp(const vec4& a, const vec4& b, f32 t) {
    assert(t >= 0.0f && t <= 1.0f);
    return a - ((a + b) * t);
  }

  vec4 min(const vec4& xyzw1, const vec4& xyzw2) {
    return {Min(xyzw1[0], xyzw2[0]), Min(xyzw1[1], xyzw2[1]), Min(xyzw1[2], xyzw2[2]), Min(xyzw1[3], xyzw2[3])};
  }

  vec4 max(const vec4& xyzw1, const vec4& xyzw2) {
    return {Max(xyzw1[0], xyzw2[0]), Max(xyzw1[1], xyzw2[1]), Max(xyzw1[2], xyzw2[2]), Max(xyzw1[3], xyzw2[3])};
  }

// Complex
// This angle represents a counter-clockwise rotation
  complex Complex(f32 angle) {
    return {cosf(angle), sinf(angle)};
  }

  b32 operator==(const complex& c1, const complex& c2) {
    return epsilonComparison(c1.r, c2.r) &&
           epsilonComparison(c1.i, c2.i);
  }

  f32 magnitudeSquared(complex c) {
    return (c.r * c.r) + (c.i * c.i);
  }

  f32 magnitude(complex c) {
    return sqrtf(magnitudeSquared(c));
  }

// ii = -1
  vec2 operator*(const complex& ri, vec2 xy) {
#ifndef NDEBUG
    // assert that we are only ever dealing with unit quaternions when rotating a vector
    f32 qMagn = magnitudeSquared(ri);
    assert(qMagn < 1.001f && qMagn > .999f);
#endif

    return {
        (ri.r * xy[0]) + -(ri.i * xy[1]), // real
        (ri.r * xy[1]) + (ri.i * xy[0]) // imaginary
    };
  }

  void operator*=(vec2& xy, const complex& ri) {
#ifndef NDEBUG
    // assert that we are only ever dealing with unit quaternions when rotating a vector
    f32 qMagn = magnitudeSquared(ri);
    assert(qMagn < 1.001f && qMagn > .999f);
#endif

    f32 imaginaryTmp = (ri.i * xy[0]);
    xy[0] = (ri.r * xy[0]) + -(ri.i * xy[1]);
    xy[1] = (ri.r * xy[1]) + imaginaryTmp;
  }

// Quaternions
  quaternion Quaternion(vec3 v, f32 angle) {
    vec3 n = normalize(v);

    f32 halfA = angle * 0.5f;
    f32 cosHalfA = cosf(halfA);
    f32 sinHalfA = sinf(halfA);

    quaternion result;
    result.r = cosHalfA;
    result.ijk = sinHalfA * n;

    return result;
  }

  b32 operator==(const quaternion& q1, const quaternion& q2) {
    return epsilonComparison(q1.r, q2.r) &&
           epsilonComparison(q1.i, q2.i) &&
           epsilonComparison(q1.j, q2.j) &&
           epsilonComparison(q1.k, q2.k);
  }

  quaternion identity_quaternion() {
    return {1.0f, 0.0f, 0.0f, 0.0f};
  }

  f32 magnitudeSquared(quaternion rijk) {
    return (rijk.r * rijk.r) + (rijk.i * rijk.i) + (rijk.j * rijk.j) + (rijk.k * rijk.k);
  }

  f32 magnitude(quaternion q) {
    return sqrtf(magnitudeSquared(q));
  }

// NOTE: Conjugate is equal to the inverse when the quaternion is unit length
  quaternion conjugate(quaternion q) {
    return {q.r, -q.i, -q.j, -q.k};
  }

  f32 dot(const quaternion& q1, const quaternion& q2) {
    return (q1.r * q2.r) + (q1.i * q2.i) + (q1.j * q2.j) + (q1.k * q2.k);
  }

  quaternion normalize(const quaternion& q) {
    f32 mag = sqrtf((q.r * q.r) + (q.i * q.i) + (q.j * q.j) + (q.k * q.k));
    assert(mag != 0.0f);
    f32 magInv = 1.0f / mag;
    return {q.r * magInv, q.i * magInv, q.j * magInv, q.k * magInv};
  }

  quaternion operator*(const quaternion& q1, const quaternion& q2) {
/*
 * Quaternion multiplication definitions
 * ii = jj = kk = ijk = -1
 * ij = -ji = k
 * jk = -kj = i
 * ki = -ik = j
 */

    quaternion q1q2;
    q1q2.r = (q1.r * q2.r) + -(q1.i * q2.i) + -(q1.j * q2.j) + -(q1.k * q2.k);
    q1q2.i = (q1.i * q2.r) + (q1.r * q2.i) + -(q1.k * q2.j) + (q1.j * q2.k);
    q1q2.j = (q1.j * q2.r) + (q1.r * q2.j) + (q1.k * q2.i) + -(q1.i * q2.k);
    q1q2.k = (q1.k * q2.r) + (q1.r * q2.k) + -(q1.j * q2.i) + (q1.i * q2.j);
    return q1q2;
  }

  vec3 rotate(const vec3& v, const quaternion& q) {
#ifndef NDEBUG
    // assert that we are only ever dealing with unit quaternions when rotating a vector
    f32 qMagn = magnitudeSquared(q);
    assert(qMagn < 1.001f && qMagn > .999f);
#endif

    quaternion result;
    quaternion qv;
    quaternion qInv = conjugate(q);

/*
 * Quaternion multiplication definitions
 * ii = jj = kk = ijk = -1
 * ij = -ji = k
 * jk = -kj = i
 * ki = -ik = j
 */

    qv.r = -(q.i * v[0]) + -(q.j * v[1]) + -(q.k * v[2]);
    qv.i = (q.r * v[0]) + -(q.k * v[1]) + (q.j * v[2]);
    qv.j = (q.r * v[1]) + (q.k * v[0]) + -(q.i * v[2]);
    qv.k = (q.r * v[2]) + -(q.j * v[0]) + (q.i * v[1]);

    // result.r = (qv.r * qInv.r) + -(qv.i * qInv.i) + -(qv.j * qInv.j) + -(qv.k * qInv.k); \\ equates to zero
    result.i = (qv.i * qInv.r) + (qv.r * qInv.i) + -(qv.k * qInv.j) + (qv.j * qInv.k);
    result.j = (qv.j * qInv.r) + (qv.k * qInv.i) + (qv.r * qInv.j) + -(qv.i * qInv.k);
    result.k = (qv.k * qInv.r) + -(qv.j * qInv.i) + (qv.i * qInv.j) + (qv.r * qInv.k);

    return result.ijk;
  }

  quaternion operator+(const quaternion& q1, const quaternion& q2) {
    return {q1.r + q2.r, q1.i + q2.i, q1.j + q2.j, q1.k + q2.k};
  }

  quaternion operator-(const quaternion& q1, const quaternion& q2) {
    return {q1.r - q2.r, q1.i - q2.i, q1.j - q2.j, q1.k - q2.k};
  }

  quaternion operator*(const quaternion& q1, const f32 s) {
    return {q1.r * s, q1.i * s, q1.j * s, q1.k * s};
  }

  quaternion operator*(const f32 s, const quaternion& q1) {
    return {q1.r * s, q1.i * s, q1.j * s, q1.k * s};
  }

  quaternion operator/(const quaternion& q1, const f32 s) {
    assert(s != 0);
    f32 scaleInv = 1.0f / s;
    return {q1.r * scaleInv, q1.i * scaleInv, q1.j * scaleInv, q1.k * scaleInv};
  }

  quaternion lerp(quaternion a, quaternion b, f32 t) {
    assert(t >= 0.0f && t <= 1.0f);
    return normalize(a - ((a + b) * t));
  }

// spherical linear interpolation
// this gives the shortest art on a 4d unit sphere
// great for smooth animations between two orientations defined by two quaternions
  quaternion slerp(quaternion a, quaternion b, f32 t) {
    assert(t >= 0.0f && t <= 1.0f);
    f32 theta = acosf(dot(a, b));

    return ((sinf(theta * (1.0f - t)) * a) + (sinf(theta * t) * b)) / sinf(theta);
  }

// TODO: There is a whole 360 degrees of rotation around the endOrientation axis that will all result in a "correct"
// TODO: end orientation as described by the function parameters.
// TODO: A reasonable approach might be ensuring the result is essentially a yaw followed by a pitch with no roll.
  quaternion orient(const vec3& startOrientation, const vec3& endOrientation) {
    // TODO: more robust handling of close orientations
    if(startOrientation == endOrientation) {
      return identity_quaternion();
    } else if(startOrientation == -endOrientation) {
      return Quaternion(vec3{0.0f, 0.0f, 1.0f}, 180.0f * RadiansPerDegree);
    }
    vec3 unitStartOrientation = normalize(startOrientation);
    vec3 unitEndOrientation = normalize(endOrientation);
    f32 theta = acosf(dot(unitStartOrientation, unitEndOrientation));
    vec3 unitPerp = normalize(cross(unitStartOrientation, unitEndOrientation));
    return Quaternion(unitPerp, theta);
  }

// mat2
  b32 operator==(const mat2& A, const mat2& B) {
    return epsilonComparison(A[0], B[0]) &&
           epsilonComparison(A[1], B[1]) &&
           epsilonComparison(A[2], B[2]) &&
           epsilonComparison(A[3], B[3]);
  }

  mat2 identity_mat2() {
    return mat2{
            1.0f, 0.0f,
            0.0f, 1.0f,
    };
  }

  mat2 scale_mat2(f32 scale) {
    return mat2{
            scale, 0.0f,
            0.0f, scale,
    };
  }

  mat2 scale_mat2(vec3 scale) {
    return mat2{
            scale[0], 0.0f,
            0.0f, scale[1],
    };
  }

  mat2 transpose(const mat2& A) {
    return mat2{
      A[0], A[2],
      A[1], A[3]
    };
  }

  mat2 rotate(f32 radians) {
    f32 sine = (f32)sin(radians);
    f32 cosine = (f32)cos(radians);
    return mat2{cosine, -sine, sine, cosine};
  }

// mat3
  b32 operator==(const mat3& A, const mat3& B) {
    return epsilonComparison(A[0], B[0]) &&
           epsilonComparison(A[1], B[1]) &&
           epsilonComparison(A[2], B[2]) &&
           epsilonComparison(A[3], B[3]) &&
           epsilonComparison(A[4], B[4]) &&
           epsilonComparison(A[5], B[5]) &&
           epsilonComparison(A[6], B[6]) &&
           epsilonComparison(A[7], B[7]) &&
           epsilonComparison(A[8], B[8]);
  }

  mat3 identity_mat3() {
    return mat3{
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
    };
  }

  mat3 scale_mat3(f32 scale) {
    return mat3{
            scale, 0.0f, 0.0f,
            0.0f, scale, 0.0f,
            0.0f, 0.0f, scale,
    };
  }

  mat3 scale_mat3(vec3 scale) {
    return mat3{
            scale[0], 0.0f, 0.0f,
            0.0f, scale[1], 0.0f,
            0.0f, 0.0f, scale[2],
    };
  }

  mat3 transpose(const mat3& A) {
    return mat3{
      A[0], A[3], A[6],
      A[1], A[4], A[7],
      A[2], A[5], A[8],
    };
  }

// radians in radians
  mat3 rotate_mat3(f32 radians, vec3 v) {
    vec3 axis(normalize(v));

    f32 const cosA = cosf(radians);
    f32 const sinA = sinf(radians);
    vec3 const axisTimesOneMinusCos = axis * (1.0f - cosA);

    mat3 rotate;
    rotate[0] = axis[0] * axisTimesOneMinusCos[0] + cosA;
    rotate[1] = axis[0] * axisTimesOneMinusCos[1] + sinA * axis[2];
    rotate[2] = axis[0] * axisTimesOneMinusCos[2] - sinA * axis[1];

    rotate[3] = axis[1] * axisTimesOneMinusCos[0] - sinA * axis[2];
    rotate[4] = axis[1] * axisTimesOneMinusCos[1] + cosA;
    rotate[5] = axis[1] * axisTimesOneMinusCos[2] + sinA * axis[0];

    rotate[6] = axis[2] * axisTimesOneMinusCos[0] + sinA * axis[1];
    rotate[7] = axis[2] * axisTimesOneMinusCos[1] - sinA * axis[0];
    rotate[8] = axis[2] * axisTimesOneMinusCos[2] + cosA;

    return rotate;
  }

  mat3 rotate_mat3(quaternion q) {
    mat3 resultMat{}; // zero out matrix
    vec3 xTransform = rotate(vec3{1.0f, 0.0f, 0.0f}, q);
    vec3 yTransform = rotate(vec3{0.0f, 1.0f, 0.0f}, q);
    vec3 zTransform = rotate(vec3{0.0f, 0.0f, 1.0f}, q);
    resultMat[0] = xTransform[0];
    resultMat[1] = xTransform[1];
    resultMat[2] = xTransform[2];
    resultMat[3] = yTransform[0];
    resultMat[4] = yTransform[1];
    resultMat[5] = yTransform[2];
    resultMat[6] = zTransform[0];
    resultMat[7] = zTransform[1];
    resultMat[8] = zTransform[2];
    return resultMat;
  }

  // vector treated as column vector
  vec3 operator*(const mat3& M, const vec3& v) {
    return vec3 {
        M[0] * v[0] + M[3] * v[1] + M[6] * v[2],
        M[1] * v[0] + M[4] * v[1] + M[7] * v[2],
        M[2] * v[0] + M[5] * v[1] + M[8] * v[2],
    };
  }

  // vector treated as row vector
  vec3 operator*(const vec3& v, const mat3& M) {
    return vec3{
        v[0] * M[0] + v[1] * M[1] + v[2] * M[2],
        v[0] * M[3] + v[1] * M[4] + v[2] * M[5],
        v[0] * M[6] + v[1] * M[7] + v[2] * M[8],
    };
  }

  mat3 operator*(const mat3& A, const mat3& B) {
    return mat3{
        // column #1
        (A[0] * B[0]) + (A[3] * B[1]) + (A[6] * B[2]),
        (A[1] * B[0]) + (A[4] * B[1]) + (A[7] * B[2]),
        (A[2] * B[0]) + (A[5] * B[1]) + (A[8] * B[2]),
        // column #1
        (A[0] * B[3]) + (A[3] * B[4]) + (A[6] * B[5]),
        (A[1] * B[3]) + (A[4] * B[4]) + (A[7] * B[5]),
        (A[2] * B[3]) + (A[5] * B[4]) + (A[8] * B[5]),
        // column #1
        (A[0] * B[6]) + (A[3] * B[7]) + (A[6] * B[8]),
        (A[1] * B[6]) + (A[4] * B[7]) + (A[7] * B[8]),
        (A[2] * B[6]) + (A[5] * B[7]) + (A[8] * B[8])
    };
  }

// mat4
  mat4 Mat4(mat3 M) {
    return mat4{
      M[0], M[1], M[2], 0.0f,
      M[3], M[4], M[5], 0.0f,
      M[6], M[7], M[8], 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f,
    };
  }

  b32 operator==(const mat4 &A, const mat4 &B) {
    return epsilonComparison(A[0], B[0]  ) &&
           epsilonComparison(A[1], B[1]  ) &&
           epsilonComparison(A[2], B[2]  ) &&
           epsilonComparison(A[3], B[3]  ) &&
           epsilonComparison(A[4], B[4]  ) &&
           epsilonComparison(A[5], B[5]  ) &&
           epsilonComparison(A[6], B[6]  ) &&
           epsilonComparison(A[7], B[7]  ) &&
           epsilonComparison(A[8], B[8]  ) &&
           epsilonComparison(A[9], B[9]  ) &&
           epsilonComparison(A[10], B[10]) &&
           epsilonComparison(A[11], B[11]) &&
           epsilonComparison(A[12], B[12]) &&
           epsilonComparison(A[13], B[13]) &&
           epsilonComparison(A[14], B[14]) &&
           epsilonComparison(A[15], B[15]);
  }

  mat4 identity_mat4() {
    return mat4{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
    };
  }

  mat4 scale_mat4(f32 scale) {
    return mat4{
            scale, 0.0f, 0.0f, 0.0f,
            0.0f, scale, 0.0f, 0.0f,
            0.0f, 0.0f, scale, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
    };
  }

  mat4 scale_mat4(vec3 scale) {
    return mat4{
            scale[0], 0.0f, 0.0f, 0.0f,
            0.0f, scale[1], 0.0f, 0.0f,
            0.0f, 0.0f, scale[2], 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
    };
  }

  mat4 translate_mat4(vec3 translation) {
    return mat4{
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            translation[0], translation[1], translation[2], 1.0f,
    };
  }

  mat4 transpose(const mat4& A) {
    return mat4{
      A[0], A[4], A[8], A[12],
      A[1], A[5], A[9], A[13],
      A[2], A[6], A[10], A[14],
      A[3], A[7], A[11], A[15],
    };
  }

// radians in radians
  mat4 rotate_xyPlane_mat4(f32 radians) {
    f32 const cosA = cosf(radians);
    f32 const sinA = sinf(radians);

    return mat4 {
        cosA, sinA, 0.0f, 0.0f,
        -sinA, cosA, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
  }

// radians in radians
  mat4 rotate_mat4(f32 radians, vec3 v) {
    vec3 axis(normalize(v));

    f32 const cosA = cosf(radians);
    f32 const sinA = sinf(radians);
    vec3 const axisTimesOneMinusCos = axis * (1.0f - cosA);

    return mat4{
        axis[0] * axisTimesOneMinusCos[0] + cosA,
        axis[0] * axisTimesOneMinusCos[1] + sinA * axis[2],
        axis[0] * axisTimesOneMinusCos[2] - sinA * axis[1],
        0,

        axis[1] * axisTimesOneMinusCos[0] - sinA * axis[2],
        axis[1] * axisTimesOneMinusCos[1] + cosA,
        axis[1] * axisTimesOneMinusCos[2] + sinA * axis[0],
        0,

        axis[2] * axisTimesOneMinusCos[0] + sinA * axis[1],
        axis[2] * axisTimesOneMinusCos[1] - sinA * axis[0],
        axis[2] * axisTimesOneMinusCos[2] + cosA,
        0,

        0.0f,
        0.0f,
        0.0f,
        1.0f,
    };
  }

  mat4 scaleTrans_mat4(const f32 scale, const vec3& translation) {
    return mat4{
            scale, 0.0f, 0.0f, 0.0f,
            0.0f, scale, 0.0f, 0.0f,
            0.0f, 0.0f, scale, 0.0f,
            translation[0], translation[1], translation[2], 1.0f,
    };
  }

  mat4 scaleTrans_mat4(const vec3& scale, const vec3& translation) {
    return mat4{
            scale[0], 0.0f, 0.0f, 0.0f,
            0.0f, scale[1], 0.0f, 0.0f,
            0.0f, 0.0f, scale[2], 0.0f,
            translation[0], translation[1], translation[2], 1.0f,
    };
  }

  mat4 scaleRotTrans_mat4(const vec3& scale, const vec3& rotAxis, const f32 angle, const vec3& translation) {
    vec3 axis(normalize(rotAxis));

    f32 const cosA = cosf(angle);
    f32 const sinA = sinf(angle);
    vec3 const axisTimesOneMinusCos = axis * (1.0f - cosA);

    return mat4{
        (axis[0] * axisTimesOneMinusCos[0] + cosA) * scale[0],
        (axis[0] * axisTimesOneMinusCos[1] + sinA * axis[2]) * scale[0],
        (axis[0] * axisTimesOneMinusCos[2] - sinA * axis[1]) * scale[0],
        0,

        (axis[1] * axisTimesOneMinusCos[0] - sinA * axis[2]) * scale[1],
        (axis[1] * axisTimesOneMinusCos[1] + cosA) * scale[1],
        (axis[1] * axisTimesOneMinusCos[2] + sinA * axis[0]) * scale[1],
        0,

        (axis[2] * axisTimesOneMinusCos[0] + sinA * axis[1]) * scale[2],
        (axis[2] * axisTimesOneMinusCos[1] - sinA * axis[0]) * scale[2],
        (axis[2] * axisTimesOneMinusCos[2] + cosA) * scale[2],
        0,

        translation[0],
        translation[1],
        translation[2],
        1.0f,
    };
  }

  mat4 scaleRotTrans_mat4(const vec3& scale, const quaternion& q, const vec3& translation) {
    vec3 xTransform = rotate(vec3{1.0f, 0.0f, 0.0f}, q) * scale[0];
    vec3 yTransform = rotate(vec3{0.0f, 1.0f, 0.0f}, q) * scale[1];
    vec3 zTransform = rotate(vec3{0.0f, 0.0f, 1.0f}, q) * scale[2];

    return mat4 {
      xTransform[0],
      xTransform[1],
      xTransform[2],
      0.0f,

      yTransform[0],
      yTransform[1],
      yTransform[2],
      0.0f,

      zTransform[0],
      zTransform[1],
      zTransform[2],
      0.0f,

      translation[0],
      translation[1],
      translation[2],
      1.0f,
    };
  }

  mat4 rotate_mat4(quaternion q) {
    vec3 xTransform = rotate(vec3{1.0f, 0.0f, 0.0f}, q);
    vec3 yTransform = rotate(vec3{0.0f, 1.0f, 0.0f}, q);
    vec3 zTransform = rotate(vec3{0.0f, 0.0f, 1.0f}, q);

    return mat4 {
        xTransform[0], xTransform[1], xTransform[2], 0.0f,
        yTransform[0], yTransform[1], yTransform[2], 0.0f,
        zTransform[0], zTransform[1], zTransform[2], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
  }

  vec4 operator*(const mat4& M, const vec4& v) {
    return vec4 {
        M[0] * v[0] + M[4] * v[1] + M[8] * v[2] + M[12] * v[3],
        M[1] * v[0] + M[5] * v[1] + M[9] * v[2] + M[13] * v[3],
        M[2] * v[0] + M[6] * v[1] + M[10] * v[2] + M[14] * v[3],
        M[3] * v[0] + M[7] * v[1] + M[11] * v[2] + M[15] * v[3],
    };
  }

  mat4 operator*(const mat4& A, const mat4& B) {
    return mat4 {
      // column #1
      A[0] * B[0] + A[4] * B[1] + A[8] * B[2] + A[12] * B[3],
      A[1] * B[0] + A[5] * B[1] + A[9] * B[2] + A[13] * B[3],
      A[2] * B[0] + A[6] * B[1] + A[10] * B[2] + A[14] * B[3],
      A[3] * B[0] + A[7] * B[1] + A[11] * B[2] + A[15] * B[3],
      // column #2
      A[0] * B[4] + A[4] * B[5] + A[8] * B[6] + A[12] * B[7],
      A[1] * B[4] + A[5] * B[5] + A[9] * B[6] + A[13] * B[7],
      A[2] * B[4] + A[6] * B[5] + A[10] * B[6] + A[14] * B[7],
      A[3] * B[4] + A[7] * B[5] + A[11] * B[6] + A[15] * B[7],
      // column #3
      A[0] * B[8] + A[4] * B[9] + A[8] * B[10] + A[12] * B[11],
      A[1] * B[8] + A[5] * B[9] + A[9] * B[10] + A[13] * B[11],
      A[2] * B[8] + A[6] * B[9] + A[10] * B[10] + A[14] * B[11],
      A[3] * B[8] + A[7] * B[9] + A[11] * B[10] + A[15] * B[11],
      // column #4
      A[0] * B[12] + A[4] * B[13] + A[8] * B[14] + A[12] * B[15],
      A[1] * B[12] + A[5] * B[13] + A[9] * B[14] + A[13] * B[15],
      A[2] * B[12] + A[6] * B[13] + A[10] * B[14] + A[14] * B[15],
      A[3] * B[12] + A[7] * B[13] + A[11] * B[14] + A[15] * B[15],
    };
  }

// real-time rendering 4.7.2
// ex: screenWidth = 20.0f, screenDist = 30.0f will provide the horizontal field of view
// for a person sitting 30 inches away from a 20 inch screen, assuming the screen is
// perpendicular to the line of sight.
// NOTE: Any units work as long as they are the same. Works for vertical and horizontal.
  f32 fieldOfView(f32 screenWidth, f32 screenDist) {
    f32 phi = 2.0f * atanf(screenWidth / (2.0f * screenDist));
    return phi;
  }

// real-time rendering 4.7.1
// This projection is for a canonical view volume goes from <-1,1>
  mat4 orthographic(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    // NOTE: Projection matrices are all about creating properly placing
    // objects in the canonical view volume, which in the case of OpenGL
    // is from <-1,-1,-1> to <1,1,1>.
    // The 3x3 matrix is dividing vectors/points by the specified dimensions
    // and multiplying by two. That is because the the canonical view volume
    // in our case actually has dimensions of <2,2,2>. So we map our specified
    // dimensions between the values of 0 and 2
    // The translation value is necessary in order to translate the values from
    // the range of 0 and 2 to the range of -1 and 1
    // The translation may look odd and that is because this matrix is a
    // combination of a Scale matrix, S, and a translation matrix, T, in
    // which the translation must be performed first. Taking the x translation
    // for example, we want to translate it by -(l + r) / 2. Which would
    // effectively move the origin's x value in the center of l & r. However,
    // the scaling changes that translation to [(2 / (r + l)) * (-(r + l) / 2)],
    // which simplifies to what is seen below for the x translation.
    return {
            2 / (r - l), 0, 0, 0,
            0, 2 / (t - b), 0, 0,
            0, 0, 2 / (f - n), 0,
            -(r + l) / (r - l), -(t + b) / (t - b), -(f + n) / (f - n), 1
    };
  }

// real-time rendering 4.7.2
  mat4 perspective(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    return mat4 {
        (2.0f * n) / (r - l), 0.0f, 0.0f, 0.0f,
        0.0f, (2.0f * n) / (t - b), 0.0f, 0.0f,
        (r + l) / (r - l), (t + b) / (t - b), -(f + n) / (f - n), -1.0f,
        0.0f, 0.0f, -(2.0f * f * n) / (f - n), 0.0f,
    };
  }

  mat4 perspectiveInverse(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    return mat4{
        (r - l) / (2.0f * n), 0.0f, 0.0f, 0.0f,
        0.0f, (t - b) / (2.0f * n), 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -(f - n) / (2 * f * n),
        (r + l) / (2.0f * n), (t + b) / (2.0f * n), -1.0f, (f + n) / (2 * f * n),
    };
  }

// real-time rendering 4.7.2
// aspect ratio is equivalent to width / height
  mat4 perspective(f32 fovVert, f32 aspect, f32 n, f32 f) {
    const f32 c = 1.0f / tanf(fovVert / 2.0f);
    return mat4{
        (c / aspect), 0.0f, 0.0f, 0.0f,
        0.0f, c, 0.0f, 0.0f,
        0.0f, 0.0f, -(f + n) / (f - n), -1.0f,
        0.0f, 0.0f, -(2.0f * f * n) / (f - n), 0.0f,
    };
  }

  mat4 perspective_fovHorz(f32 fovHorz, f32 aspect, f32 n, f32 f) {
    const f32 c = 1.0f / tanf(fovHorz / 2.0f);
    return mat4{
        c, 0.0f, 0.0f, 0.0f,
        0.0f, (c / (1.0f / aspect)), 0.0f, 0.0f,
        0.0f, 0.0f, -(f + n) / (f - n), -1.0f,
        0.0f, 0.0f, -(2.0f * f * n) / (f - n), 0.0f,
    };
  }


  mat4 perspectiveInverse(f32 fovVert, f32 aspect, f32 n, f32 f) {
    const f32 c = 1.0f / tanf(fovVert / 2.0f);
    return mat4{
        aspect / c, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f / c, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -(f - n) / (2.0f * f * n),
        0.0f, 0.0f, -1.0f, (f + n) / (2.0f * f * n),
    };
  }

/*
 * // NOTE: Oblique View Frustum Depth Projection and Clipping by Eric Lengyel (Terathon Software)
 * Arguments:
 * mat4 perspectiveMat: Perspective matrix that is being used for the rest of the scene
 * vec3 planeNormal_viewSpace: Plane normal in view space. MUST be normalized. This is a normal that points INTO the frustum, NOT one that is generally pointing towards the camera.
 * vec3 planePos_viewSpace: Any position on the plane in view space.
 */
  mat4 obliquePerspective(const mat4& perspectiveMat, vec3 planeNormal_viewSpace, vec3 planePos_viewSpace, f32 farPlane) {
    // plane = {normal[0], normal[1], normal[2], dist}
    vec4 plane_viewSpace = Vec4(planeNormal_viewSpace, dot(-planeNormal_viewSpace, planePos_viewSpace));

    // clip space plane
    vec4 oppositeFrustumCorner_viewSpace = {
            sign(plane_viewSpace[0]) * (1.0f / perspectiveMat[0]),
            sign(plane_viewSpace[1]) * (1.0f / perspectiveMat[5]),
            -1.0f,
            1.0f / farPlane
    };

    vec4 scaledPlane_viewSpace = ((-2.0f * oppositeFrustumCorner_viewSpace[2]) / dot(plane_viewSpace, oppositeFrustumCorner_viewSpace)) * plane_viewSpace;

    return mat4{
      0.0f, 0.0f, scaledPlane_viewSpace[0], 0.0f,
      0.0f, 0.0f, scaledPlane_viewSpace[1], 0.0f,
      0.0f, 0.0f, scaledPlane_viewSpace[2] + 1.0f, 0.0f,
      0.0f, 0.0f, scaledPlane_viewSpace[3], 0.0f,
    };
  }

/*
 * // NOTE: Oblique View Frustum Depth Projection and Clipping by Eric Lengyel (Terathon Software)
 * Arguments:
 * vec3 planeNormal_viewSpace: Plane normal in view space. MUST be normalized. This is a normal that points INTO the frustum, NOT one that is generally pointing towards the camera.
 * vec3 planePos_viewSpace: Any position on the plane in view space.
 */
  mat4 obliquePerspective(f32 fovVert, f32 aspect, f32 nearPlane, f32 farPlane, vec3 planeNormal_viewSpace, vec3 planePos_viewSpace) {
    const f32 c = 1.0f / tanf(fovVert / 2.0f);

    // plane = {normal[0], normal[1], normal[2], dist}
    vec4 plane_viewSpace = Vec4(planeNormal_viewSpace, dot(-planeNormal_viewSpace, planePos_viewSpace));

    // clip space plane
    vec4 oppositeFrustumCorner_viewSpace = {
            sign(plane_viewSpace[0]) * (aspect / c),
            sign(plane_viewSpace[1]) * (1.0f / c),
            -1.0f,
            1.0f / farPlane
    };

    vec4 scaledPlane_viewSpace = ((-2.0f * oppositeFrustumCorner_viewSpace[2]) / dot(plane_viewSpace, oppositeFrustumCorner_viewSpace)) * plane_viewSpace;

    mat4 resultMat = {
            (c / aspect), 0.0f, scaledPlane_viewSpace[0], 0.0f,
            0.0f, c, scaledPlane_viewSpace[1], 0.0f,
            0.0f, 0.0f, scaledPlane_viewSpace[2] + 1.0f, -1.0f,
            0.0f, 0.0f, scaledPlane_viewSpace[3], 0.0f
    };

    return resultMat;
  }

  mat4 obliquePerspective_fovHorz(f32 fovHorz, f32 aspect, f32 nearPlane, f32 farPlane, vec3 planeNormal_viewSpace, vec3 planePos_viewSpace) {
    const f32 c = 1.0f / tanf(fovHorz / 2.0f);

    // plane = {normal[0], normal[1], normal[2], dist}
    vec4 plane_viewSpace = Vec4(planeNormal_viewSpace, dot(-planeNormal_viewSpace, planePos_viewSpace));

    // clip space plane
    vec4 oppositeFrustumCorner_viewSpace = {
        sign(plane_viewSpace[0]) * (aspect / c),
        sign(plane_viewSpace[1]) * (1.0f / c),
        -1.0f,
        1.0f / farPlane
    };

    vec4 scaledPlane_viewSpace = ((-2.0f * oppositeFrustumCorner_viewSpace[2]) / dot(plane_viewSpace, oppositeFrustumCorner_viewSpace)) * plane_viewSpace;

    mat4 resultMat = {
        c, 0.0f, scaledPlane_viewSpace[0], 0.0f,
        0.0f, (c / (1.0f / aspect)), scaledPlane_viewSpace[1], 0.0f,
        0.0f, 0.0f, scaledPlane_viewSpace[2] + 1.0f, -1.0f,
        0.0f, 0.0f, scaledPlane_viewSpace[3], 0.0f
    };

    return resultMat;
  }

  void adjustAspectPerspProj(mat4* projectionMatrix, f32 fovVert, f32 aspect) {
    const f32 c = 1.0f / tanf(fovVert / 2.0f);
    (*projectionMatrix)[0] = c / aspect;
    (*projectionMatrix)[5] = c;
  }

  void adjustNearFarPerspProj(mat4* projectionMatrix, f32 n, f32 f) {
    (*projectionMatrix)[10] = -(f + n) / (f - n);
    (*projectionMatrix)[14] = -(2.0f * f * n) / (f - n);
  }

// etc
  bool insideRect(BoundingRect boundingRect, vec2 position) {
    const vec2 boundingRectMax = boundingRect.min + boundingRect.diagonal;
    return (position[0] > boundingRect.min[0] &&
            position[0] < boundingRectMax[0] &&
            position[1] > boundingRect.min[1] &&
            position[1] < boundingRectMax[1]);
  }

  bool insideBox(BoundingBox boundingBox, vec3 position) {
    const vec3 boundingBoxMax = boundingBox.min + boundingBox.diagonal;
    return (position[0] > boundingBox.min[0] &&
            position[0] < boundingBoxMax[0] &&
            position[1] > boundingBox.min[1] &&
            position[1] < boundingBoxMax[1] &&
            position[2] > boundingBox.min[2] &&
            position[2] < boundingBoxMax[2]);
  }

  bool overlap(BoundingRect bbA, BoundingRect bbB) {
    const vec2 bbAMax = bbA.min + bbA.diagonal;
    const vec2 bbBMax = bbB.min + bbB.diagonal;
    return ((bbBMax[0] > bbA.min[0] && bbB.min[0] < bbAMax[0]) && // overlap in X
            (bbBMax[1] > bbA.min[1] && bbB.min[1] < bbAMax[0]));   // overlap in Y
  }

  bool overlap(BoundingBox bbA, BoundingBox bbB) {
    const vec3 bbAMax = bbA.min + bbA.diagonal;
    const vec3 bbBMax = bbB.min + bbB.diagonal;
    return ((bbBMax[0] > bbA.min[0] && bbB.min[0] < bbAMax[0]) && // overlap in X
            (bbBMax[1] > bbA.min[1] && bbB.min[1] < bbAMax[1]) && // overlap in X
            (bbBMax[2] > bbA.min[2] && bbB.min[2] < bbAMax[2]));   // overlap in Z
  }
}
