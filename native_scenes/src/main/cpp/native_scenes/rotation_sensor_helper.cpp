struct RotationSensorHelper{
  ASensorEventQueue* sensorEventQueue;
  const ASensor* gameRotationSensor = nullptr;
  vec4 lastRotationVector = {0.0f, 0.0f, 0.0f, 0.0f };
  mat3 firstRotationMat = identity_mat3();

  void init(ASensorManager* sensorManager, ASensorEventQueue* sensorEventQueue) {
    this->sensorEventQueue = sensorEventQueue;
    gameRotationSensor = ASensorManager_getDefaultSensor(sensorManager,
                                                         ASENSOR_TYPE_GAME_ROTATION_VECTOR);
  }

  void pause() {
    // Stop monitoring the accelerometer to avoid consuming battery while not being used.
    if (gameRotationSensor != nullptr) {
      ASensorEventQueue_disableSensor(sensorEventQueue, gameRotationSensor);
    }
  }

  void resume() {
    if (gameRotationSensor != nullptr) {
      ASensorEventQueue_enableSensor(sensorEventQueue, gameRotationSensor);
      const s32 microsecondsPerSecond = 1000000;
      const s32 samplesPerSecond = 60;
      const s32 microsecondsPerSample = microsecondsPerSecond / samplesPerSecond;
      ASensorEventQueue_setEventRate(sensorEventQueue, gameRotationSensor,
                                     microsecondsPerSample);
    }
  }

  // returns true if event was relevant and processed. False otherwise.
  bool processEvent(const ASensorEvent& sensorEvent) {
    if(sensorEvent.type == ASENSOR_TYPE_GAME_ROTATION_VECTOR && sensorEvent.data[3] != 1.0) {
      LOGI("game rotation vector: inputX=%f inputY=%f z=%f", sensorEvent.data[0], sensorEvent.data[1], sensorEvent.data[2]);
      lastRotationVector = vec4{sensorEvent.data[0], sensorEvent.data[1], sensorEvent.data[2], sensorEvent.data[3]};
      return true;
    } else { return false; }
  }

  mat3 getRotationMat() {
    if (lastRotationVector[0] == 0 &&
        lastRotationVector[1] == 0 &&
        lastRotationVector[2] == 0 &&
        lastRotationVector[3] == 0) {
      // If the sensor hasn't given us anything our rotation is an identity matrix
      return identity_mat3();
    } else if (firstRotationMat[0] == 1.0f &&
               firstRotationMat[4] == 1.0f &&
               firstRotationMat[8] == 1.0f) {
      firstRotationMat = getRotationMatrixFromVector(lastRotationVector);
      return identity_mat3();
    } else {
      mat3 lastRotationMat = getRotationMatrixFromVector(lastRotationVector);
      return transpose(lastRotationMat) * firstRotationMat; // transpose is inverse for pure rotation matrix
    }
  }

private:
/**
 * translated from android.hardware.SensorManager.java::getRotationMatrixFromVector
 */
  static mat3 getRotationMatrixFromVector(const vec4& rotationVector) {
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
};

