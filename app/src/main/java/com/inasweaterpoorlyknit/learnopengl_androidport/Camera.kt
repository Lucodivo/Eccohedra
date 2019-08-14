package com.inasweaterpoorlyknit.learnopengl_androidport

import android.opengl.Matrix
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.radians
import kotlin.math.cos
import kotlin.math.sin

enum class CameraMovement {
    Forward,
    Backward,
    Left,
    Right,
    Jump
}

const val PITCH = 0.0f
const val YAW = -90.0f
const val SPEED = 2.5f
const val SENSITIVTY = 0.1f
const val ZOOM = 45.0f
const val JUMP_SPEED = 1

class Camera {
    private val position: Vec3 = Vec3(0.0f, 0.0f, 3.0f)
    private var up: Vec3 = Vec3(0.0f, 1.0f, 0.0f)
    private val yaw: Float = YAW
    private val pitch: Float = PITCH

    private var front = Vec3(0.0f, 0.0f, -1.0f)
    private var right = Vec3(1.0f, 0.0f, 0.0f)
    private var worldUp = Vec3(0.0f, 1.0f, 0.0f)

    var movementSpeed = SPEED
    var zoom = ZOOM

    var deltaPosition = Vec3(0.0f, 0.0f, 0.0f)

    init {
        updateCameraVectors()
    }

    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    fun getViewMatrix(deltaTime: Float) : FloatArray
    {
        return lookAt()
    }

    // TODO: Test that the transpose logic is necessary
    private fun lookAt() : FloatArray
    {
        val target = position + front

        // Calculate cameraDirection
        val zAxis = (position - target).normalize()
        // Get positive right axis vector
        // TODO: Is the inner normalize of up necessary?
        val xAxis = cross(up.normalize(), zAxis).normalize()
        // Calculate camera up vector
        val yAxis = cross(zAxis, xAxis)

        // In glm we access elements as mat[col][row] due to column-major layout
        val translation = floatArrayOf(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -position.x, -position.y, -position.z, 1.0f)
        val translationTranspose = FloatArray(translation.size)
        Matrix.transposeM(translationTranspose, 0, translation, 0)

        val rotation = floatArrayOf(
            xAxis.x, yAxis.x, zAxis.x, 0.0f,
            xAxis.y, yAxis.y, zAxis.y, 0.0f,
            xAxis.z, yAxis.z, zAxis.z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f)
        val rotationTranspose = FloatArray(rotation.size)
        Matrix.transposeM(rotationTranspose, 0, rotation, 0)

        // Return lookAt matrix as combination of translation and rotation matrix
        val resultMatrix = FloatArray(rotation.size)
        // Remember to read from right to left (first translation then rotation)
        Matrix.multiplyMM(resultMatrix, 0, rotation, 0, translation, 0)
        return resultMatrix
    }

    // Calculates the front vector from the Camera's (updated) Eular Angles
    private fun updateCameraVectors() {
        // Calculate the new front vector
        val newFront = Vec3(cos(radians(yaw)) * cos(radians(pitch)),
                            sin(radians(pitch)),
                        sin(radians(yaw)) * cos(radians(pitch)))
        front = newFront.normalize()
        // Also re-calculate the right and Up vector
        right = cross(front, worldUp).normalize()  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        up = cross(right, front).normalize()
    }
}