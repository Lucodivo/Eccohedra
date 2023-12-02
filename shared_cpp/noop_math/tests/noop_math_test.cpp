#include "test.h"

#define cos30 0.86602540f
#define cos45 0.70710678f
#define cos60 0.5f

#define sin30 0.5f
#define sin45 0.70710678f
#define sin60 0.86602540f


TEST(NoopMath, translateTest) {
  vec3 translation{12.8f, 25.6f, 51.2f};
  mat4 expectedResult{
          1.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 1.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 1.0f, 0.0f,
          translation[0], translation[1], translation[2], 1.0f
  };
  mat4 translatedMat = translate_mat4(translation);

  ASSERT_TRUE(printIfNotEqual(expectedResult, translatedMat));
}

TEST(NoopMath, mat4Vec4MultTest) {
  vec4 point{1, 2, 3, 1};
  vec4 vector{1, 2, 3, 0};
  vec3 translation{7.0f, 8.0f, 9.0f};

  vec4 expectedPoint{point[0] + translation[0], point[1] + translation[1], point[2] + translation[2], point[3]};
  vec4 expectedVector = vector;
  mat4 M = translate_mat4(translation);

  vec4 translatedPoint = M * point;
  vec4 translatedVector = M * vector;

  ASSERT_TRUE(printIfNotEqual(translatedVector, expectedVector));
  ASSERT_TRUE(printIfNotEqual(translatedPoint, expectedPoint));
}

TEST(NoopMath, mat4MultTest) {
  vec4 startingPos{1.0f, 10.0f, 100.0f, 1.0f};
  vec3 scale{1.0f, 2.0f, 3.0f};
  vec3 translation{7.0f, 8.0f, 9.0f};
  mat4 scaleMat4 = scale_mat4(scale);
  mat4 translateMat4 = translate_mat4(translation);

  vec4 expectedPosTransThenScale{scale[0] * (startingPos[0] + translation[0]),
                                 scale[1] * (startingPos[1] + translation[1]),
                                 scale[2] * (startingPos[2] + translation[2]),
                                 1.0f};

  vec4 expectedPosScaleThenTrans{(scale[0] * startingPos[0]) + translation[0],
                                 (scale[1] * startingPos[1]) + translation[1],
                                 (scale[2] * startingPos[2]) + translation[2],
                                 1.0f};

  vec4 transformedPosTransThenScale = scaleMat4 * translateMat4 * startingPos;
  vec4 transformedPosScaleThenTrans = translateMat4 * scaleMat4 * startingPos;

  ASSERT_TRUE(printIfNotEqual(transformedPosScaleThenTrans, expectedPosScaleThenTrans));
  ASSERT_TRUE(printIfNotEqual(transformedPosTransThenScale, expectedPosTransThenScale));
}

TEST(NoopMath, mat4RotateTest) {
  f32 angle = 120.f * RadiansPerDegree;
  vec3 rotationAxis{1.0f, 1.0f, 1.0f};
  vec4 x{1.0f, 0.0f, 0.0f, 1.0f};
  vec4 y{0.0f, 1.0f, 0.0f, 1.0f};
  vec4 z{0.0f, 0.0f, 1.0f, 1.0f};

  mat4 rotationMat = rotate_mat4(angle, rotationAxis);

  vec4 rotatedX = rotationMat * x;
  vec4 rotatedY = rotationMat * y;
  vec4 rotatedZ = rotationMat * z;
  vec4 rotatedAxis = rotationMat * vec4{rotationAxis[0], rotationAxis[1], rotationAxis[2], 1.0f};

  ASSERT_EQ(rotatedX, y);
  ASSERT_EQ(rotatedY, z);
  ASSERT_EQ(rotatedZ, x);
  ASSERT_EQ(rotatedAxis.xyz, rotationAxis);
}

TEST(NoopMath, complexVec2RotationTest) {
  vec2 x{1.0f, 0.0f};
  vec2 y{0.0f, 1.0f};
  complex c0 = Complex(0.0f);
  complex c30 = Complex(30.0f * RadiansPerDegree);
  complex c45 = Complex(45.0f * RadiansPerDegree);
  complex c60 = Complex(60.0f * RadiansPerDegree);
  complex c90 = Complex(90.0f * RadiansPerDegree);
  vec2 expectedRotatedX0 = x;
  vec2 expectedRotatedX30 = {cos30, sin30};
  vec2 expectedRotatedX45 = {cos45, sin45};
  vec2 expectedRotatedX60 = {cos60, sin60};
  vec2 expectedRotatedX90 = y;
  vec2 expectedRotatedY0 = y;
  vec2 expectedRotatedY30 = {-sin30, cos30};
  vec2 expectedRotatedY45 = {-sin45, cos45};
  vec2 expectedRotatedY60 = {-sin60, cos60};
  vec2 expectedRotatedY90 = -x;

  vec2 rotatedX0 = c0 * x;
  vec2 rotatedX30 = c30 * x;
  vec2 rotatedX45 = c45 * x;
  vec2 rotatedX60 = c60 * x;
  vec2 rotatedX90 = c90 * x;
  vec2 rotatedY0 = c0 * y;
  vec2 rotatedY30 = c30 * y;
  vec2 rotatedY45 = c45 * y;
  vec2 rotatedY60 = c60 * y;
  vec2 rotatedY90 = c90 * y;

  ASSERT_EQ(rotatedX0, expectedRotatedX0);
  ASSERT_EQ(rotatedX30, expectedRotatedX30);
  ASSERT_EQ(rotatedX45, expectedRotatedX45);
  ASSERT_EQ(rotatedX60, expectedRotatedX60);
  ASSERT_EQ(rotatedX90, expectedRotatedX90);
  ASSERT_EQ(rotatedY0, expectedRotatedY0);
  ASSERT_EQ(rotatedY30, expectedRotatedY30);
  ASSERT_EQ(rotatedY45, expectedRotatedY45);
  ASSERT_EQ(rotatedY60, expectedRotatedY60);
  ASSERT_EQ(rotatedY90, expectedRotatedY90);
}

TEST(NoopMath, quaternionVec3RotationTest) {
  f32 angle = 120.f * RadiansPerDegree;
  vec3 rotationAxis{1.0f, 1.0f, 1.0f};
  vec3 x{1.0f, 0.0f, 0.0f};
  vec3 y{0.0f, 1.0f, 0.0f};
  vec3 z{0.0f, 0.0f, 1.0f};

  quaternion q = Quaternion(rotationAxis, angle);

  vec3 rotatedX = rotate(x, q);
  vec3 rotatedY = rotate(y, q);
  vec3 rotatedZ = rotate(z, q);
  vec3 rotatedAxis = rotate(rotationAxis, q);

  ASSERT_EQ(rotatedX, y); // x => y
  ASSERT_EQ(rotatedY, z); // y => z
  ASSERT_EQ(rotatedZ, x); // z => x
  ASSERT_EQ(rotatedAxis, rotationAxis); // rotation axis unchanged
}

TEST(NoopMath, crossProductTest) {
  vec3 x{1.0f, 0.0f, 0.0f};
  vec3 y{0.0f, 1.0f, 0.0f};
  vec3 z{0.0f, 0.0f, 1.0f};

  vec3 xCrossY = cross(x, y);
  vec3 xCrossZ = cross(x, z);
  vec3 yCrossZ = cross(y, z);
  vec3 yCrossX = cross(y, x);
  vec3 zCrossX = cross(z, x);
  vec3 zCrossY = cross(z, y);

  ASSERT_EQ(xCrossY, z);
  ASSERT_EQ(xCrossZ, -y);
  ASSERT_EQ(yCrossZ, x);
  ASSERT_EQ(yCrossX, -z);
  ASSERT_EQ(zCrossX, y);
  ASSERT_EQ(zCrossY, -x);
}

TEST(NoopMath, slerpTest) {
  vec3 rotationAxis{0.0f, 0.0f, 1.0f};
  quaternion q1 = identity_quaternion();
  quaternion q2 = Quaternion(rotationAxis, RadiansPerDegree * 90);
  vec3 vector{1.0f, 0.0f, 0.0f};
  vec3 expectedVectorZero = vector;
  vec3 expectedVectorThirtyOverNinety{cos30, sin30, 0.0f};
  vec3 expectedVectorHalf{cos45, sin45, 0.0f};
  vec3 expectedSixetyOverNinety{cos60, sin60, 0.0f};
  vec3 expectedVectorOne{0.0f, 1.0f, 0.0f};

  quaternion qZero = slerp(q1, q2, 0.0f);
  quaternion qThirtyOverNinety = slerp(q1, q2, 30.0f / 90.0f);
  quaternion qHalf = slerp(q1, q2, 0.5f);
  quaternion qSixetyOverNinety = slerp(q1, q2, 60.0f / 90.0f);
  quaternion qOne = slerp(q1, q2, 1.0f);
  vec3 vectorZero = rotate(vector, qZero);
  vec3 vectorThirtyOver90 = rotate(vector, qThirtyOverNinety);
  vec3 vectorHalf = rotate(vector, qHalf);
  vec3 vectorSixetyOverNinety = rotate(vector, qSixetyOverNinety);
  vec3 vectorOne = rotate(vector, qOne);

  ASSERT_EQ(vectorZero, expectedVectorZero);
  ASSERT_EQ(vectorThirtyOver90, expectedVectorThirtyOverNinety);
  ASSERT_EQ(vectorHalf, expectedVectorHalf);
  ASSERT_EQ(vectorSixetyOverNinety, expectedSixetyOverNinety);
  ASSERT_EQ(vectorOne, expectedVectorOne);

  // also test counter-clockwise slerp works as intended
  quaternion q3 = Quaternion(rotationAxis, RadiansPerDegree * -90);
  expectedVectorZero = vector;
  expectedVectorThirtyOverNinety = {cos30, -sin30, 0.0f};
  expectedVectorHalf = {cos45, -sin45, 0.0f};
  expectedSixetyOverNinety = {cos60, -sin60, 0.0f};
  expectedVectorOne = {0.0f, -1.0f, 0.0f};

  qZero = slerp(q1, q3, 0.0f);
  qThirtyOverNinety = slerp(q1, q3, 30.0f / 90.0f);
  qHalf = slerp(q1, q3, 0.5f);
  qSixetyOverNinety = slerp(q1, q3, 60.0f / 90.0f);
  qOne = slerp(q1, q3, 1.0f);
  vectorZero = rotate(vector, qZero);
  vectorThirtyOver90 = rotate(vector, qThirtyOverNinety);
  vectorHalf = rotate(vector, qHalf);
  vectorSixetyOverNinety = rotate(vector, qSixetyOverNinety);
  vectorOne = rotate(vector, qOne);

  ASSERT_EQ(vectorZero, expectedVectorZero);
  ASSERT_EQ(vectorThirtyOver90, expectedVectorThirtyOverNinety);
  ASSERT_EQ(vectorHalf, expectedVectorHalf);
  ASSERT_EQ(vectorSixetyOverNinety, expectedSixetyOverNinety);
  ASSERT_EQ(vectorOne, expectedVectorOne);
}

TEST(NoopMath, orthographicTest) {
  vec4 point{15.0f, 70.0f, 300.0f, 1.0f};
  f32 l = -20.0f;
  f32 r = 40.0f;
  f32 b = 50.0f;
  f32 t = 110.0f;
  f32 n = 230.0f;
  f32 f = 500.0f;
  vec4 expectedCanonicalViewPoint{
          (point[0] - ((r + l) / 2.0f)) // move origin to center
          * (2.0f / (r - l)), // dimens desired dimens between -1 and 1
          (point[1] - ((t + b) / 2.0f)) // move origin to center
          * (2.0f / (t - b)), // dimens desired dimens between -1 and 1
          (point[2] - ((f + n) / 2.0f)) // move origin to center
          * (2.0f / (f - n)), // dimens desired dimens between -1 and 1
          1.0f
  };
  mat4 ortho = orthographic(l, r, b, t, n, f);

  vec4 transformedPoint = ortho * point;

  ASSERT_EQ(transformedPoint, expectedCanonicalViewPoint);
}

TEST(NoopMath, bracketAssignmentOperatorsSanityCheck) {
  f32 zanyWhackyNum = 123.456f;
  u32 indexOfInterest = 1;
  vec2 v2{};
  vec3 v3{};
  vec4 v4{};
  mat3 m3{};
  mat4 m4{};

  v2[indexOfInterest] = zanyWhackyNum;
  v3[indexOfInterest] = zanyWhackyNum;
  v4[indexOfInterest] = zanyWhackyNum;
  m3[indexOfInterest] = zanyWhackyNum;
  m4[indexOfInterest] = zanyWhackyNum;

  ASSERT_EQ(v2[indexOfInterest], zanyWhackyNum);
  ASSERT_EQ(v3[indexOfInterest], zanyWhackyNum);
  ASSERT_EQ(v4[indexOfInterest], zanyWhackyNum);
  ASSERT_EQ(m3[indexOfInterest], zanyWhackyNum);
  ASSERT_EQ(m4[indexOfInterest], zanyWhackyNum);
}

TEST(NoopMath, quaternionOrientTest) {
  vec3 orientation1 = normalize(1.0f, 1.0f, 1.0f);
  vec3 orientation2 = normalize(1.0, 3.0f, -6.0f);
  quaternion orientQuaternion1to2 = orient(orientation1, orientation2);
  quaternion orientQuaternion2to1 = orient(orientation2, orientation1);
  mat3 matrix1to2 = rotate_mat3(orientQuaternion1to2);
  mat3 matrix2to1 = rotate_mat3(orientQuaternion2to1);

  vec3 resultOrientationQuaternion1to2 = rotate(orientation1, orientQuaternion1to2);
  vec3 resultOrientationQuaternion2to1 = rotate(orientation2, orientQuaternion2to1);
  vec3 resultOrientationMat1to2 = matrix1to2 * orientation1;
  vec3 resultOrientationMat2to1 = matrix2to1 * orientation2;

  ASSERT_EQ(resultOrientationQuaternion1to2, orientation2);
  ASSERT_EQ(resultOrientationQuaternion2to1, orientation1);
  ASSERT_EQ(resultOrientationMat1to2, orientation2);
  ASSERT_EQ(resultOrientationMat2to1, orientation1);
}

TEST(NoopMath, inversePerspectiveTests) {
  f32 l = 10.0f;
  f32 r = 30.0f;
  f32 b = -50.0f;
  f32 t = 50.0f;
  f32 n = 1.0f;
  f32 f = 70.0f;
  f32 fovy = fieldOfView(13.5f, 25.0f);
  f32 aspect = 1920.0f / 1080.0f;
  mat4 perspLRBT = perspective(l, r, b, t, n, f);
  mat4 perspFOV = perspective(fovy, aspect, n, f);
  mat4 inverseLRBT = perspectiveInverse(l, r, b, t, n, f);
  mat4 inverseFOV = perspectiveInverse(fovy, aspect, n, f);

  mat4 shouldBeIdentityLRBT = inverseLRBT * perspLRBT;
  ASSERT_TRUE(printIfNotEqual(shouldBeIdentityLRBT, identity_mat4()));
  mat4 shouldBeIdentityFOV = inverseFOV * perspFOV;
  ASSERT_TRUE(printIfNotEqual(shouldBeIdentityFOV, identity_mat4()));
}
