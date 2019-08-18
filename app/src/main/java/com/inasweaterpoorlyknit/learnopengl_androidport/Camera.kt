package com.inasweaterpoorlyknit.learnopengl_androidport

import kotlin.math.cos
import kotlin.math.sin

import glm_.vec3.Vec3
import glm_.mat4x4.Mat4
import glm_.glm
import glm_.vec2.Vec2

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
const val ZOOM = 45.0f
const val MOVEMENT_SPEED = 1.0f
const val YAW_PAN_SENSITIVITY = 0.1f

class Camera {
    private val position: Vec3 = Vec3(0.0f, 0.0f, 3.0f)
    private var up: Vec3 = Vec3(0.0f, 1.0f, 0.0f)
    private var yaw: Float = YAW
    private var pitch: Float = PITCH

    private var front = Vec3(0.0f, 0.0f, -1.0f)
    private var right = Vec3(1.0f, 0.0f, 0.0f)
    private var worldUp = Vec3(0.0f, 1.0f, 0.0f)

    var zoom = ZOOM

    var deltaPosition = Vec3(0.0f, 0.0f, 0.0f)

    init {
        updateCameraVectors()
    }

    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    fun getViewMatrix(deltaTime: Float) : Mat4
    {
        changePositioning()
        updateCameraVectors()

        return lookAt()
    }

    private fun lookAt() : Mat4
    {
        val target = position + front

        // Calculate cameraDirection
        val zAxis = glm.normalize(position - target)
        // Get positive right axis vector
        val xAxis = glm.normalize(glm.cross(glm.normalize(up), zAxis))
        // Calculate camera up vector
        val yAxis = glm.cross(zAxis, xAxis)

        // In glm we access elements as mat[col][row] due to column-major layout
        val translation = Mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -position.x, -position.y, -position.z, 1.0f)

        val rotation = Mat4(
            xAxis.x, yAxis.x, zAxis.x, 0.0f,
            xAxis.y, yAxis.y, zAxis.y, 0.0f,
            xAxis.z, yAxis.z, zAxis.z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f)

        // Return lookAt matrix as combination of translation and rotation matrix
        // Remember to read from right to left (first translation then rotation)
        return rotation * translation
    }

    // Calculates the front vector from the Camera's (updated) Eular Angles
    private fun updateCameraVectors() {
        // Calculate the new front vector
        val newFront = Vec3(cos(glm.radians(yaw)) * cos(glm.radians(pitch)),
                            sin(glm.radians(pitch)),
                        sin(glm.radians(yaw)) * cos(glm.radians(pitch)))
        front = glm.normalize(newFront)
        // Also re-calculate the right and Up vector
        right = glm.normalize(glm.cross(front, worldUp))  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        up = glm.normalize(glm.cross(right, front))
    }

    fun changePositioning()
    {
        // multiplying a vec3(0,0,0) by small fractions may lead to NAN values
        if (deltaPosition.x != 0.0f || deltaPosition.y != 0.0f || deltaPosition.z != 0.0f)
        {
            position += deltaPosition
        }
        deltaPosition = Vec3(0.0f)
    }

    fun processPan(vec2: Vec2) {
        // NOTE: Uncomment to move whole scene left/right/closer/further
        //deltaPosition = Vec3(-vec2.x, 0.0f, vec2.y) * (MOVEMENT_SPEED/200.0f)
        deltaPosition = front * -vec2.y * (MOVEMENT_SPEED/200.0f)
        yaw += vec2.x * YAW_PAN_SENSITIVITY
    }
}