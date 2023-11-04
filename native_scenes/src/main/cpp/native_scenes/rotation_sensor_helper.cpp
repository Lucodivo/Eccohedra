/**
 * translated from android.hardware.SensorManager.java::getRotationMatrixFromVector
 */
mat3 getRotationMatrixFromVector(const vec4& rotationVector) {
  mat3 result;

  f32 q0 = rotationVector[3];
  f32 q1 = rotationVector[0];
  f32 q2 = rotationVector[1];
  f32 q3 = rotationVector[2];

  f32 sq_q1 = 2 * q1 * q1;
  f32 sq_q2 = 2 * q2 * q2;
  f32 sq_q3 = 2 * q3 * q3;
  f32 q1_q2 = 2 * q1 * q2;
  f32 q3_q0 = 2 * q3 * q0;
  f32 q1_q3 = 2 * q1 * q3;
  f32 q2_q0 = 2 * q2 * q0;
  f32 q2_q3 = 2 * q2 * q3;
  f32 q1_q0 = 2 * q1 * q0;

  result[0] = 1 - sq_q2 - sq_q3;
  result[3] = q1_q2 - q3_q0;
  result[6] = q1_q3 + q2_q0;

  result[1] = q1_q2 + q3_q0;
  result[4] = 1 - sq_q1 - sq_q3;
  result[7] = q2_q3 - q1_q0;

  result[2] = q1_q3 - q2_q0;
  result[5] = q2_q3 + q1_q0;
  result[8] = 1 - sq_q1 - sq_q2;

  return result;
}