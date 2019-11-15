package com.inasweaterpoorlyknit.learnopengl_androidport

import glm_.vec3.Vec3
import glm_.mat4x4.Mat4
import glm_.glm
import glm_.vec2.Vec2
import glm_.vec4.Vec4

const val PITCH = 0.0f
const val YAW = -90.0f
const val ZOOM = 45.0f

class Camera {
    val position: Vec3 = Vec3(0.0f, 0.0f, 3.0f)
    var movementSpeed = 1.0f
    private var up: Vec3 = Vec3(0.0f, 1.0f, 0.0f)
    private var yaw: Float = YAW
    private var pitch: Float = PITCH

    private var front = Vec3(0.0f, 0.0f, -1.0f)
    private var right = Vec3(1.0f, 0.0f, 0.0f)
    private var worldUp = Vec3(0.0f, 1.0f, 0.0f)

    var deltaPosition = Vec3(0.0f, 0.0f, 0.0f)

    var rotMat = Mat4(1.0f)

    // startingInverse Matrix
    lateinit var startingMat4Inverse: Mat4

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

    fun getRotationMatrix(deltaTime: Float) : Mat4
    {
        changePositioning()
        updateCameraVectors()

        return lookAtRotationMatrix()
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

    private fun lookAtRotationMatrix(): Mat4 {

        val target = position + front

        // Calculate cameraDirection
        val zAxis = glm.normalize(position - target)
        // Get positive right axis vector
        val xAxis = glm.normalize(glm.cross(glm.normalize(up), zAxis))
        // Calculate camera up vector
        val yAxis = glm.cross(zAxis, xAxis)

        val rotation = Mat4(
            xAxis.x, yAxis.x, -zAxis.x, 0.0f,
            xAxis.y, yAxis.y, -zAxis.y, 0.0f,
            xAxis.z, yAxis.z, -zAxis.z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f)

        return rotation
    }

    // Calculates the front vector from the Camera's (updated) Eular Angles
    private fun updateCameraVectors() {
        // Calculate the new front vector
        val newFront = rotMat * Vec4(0.0f, 0.0f, -1.0f, 1.0f)

        front = glm.normalize(newFront.toVec3())
        // Also re-calculate the right and Up vector
        right = glm.normalize(glm.cross(front, worldUp))  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        up = glm.normalize(glm.cross(right, front))
    }

    private fun changePositioning()
    {
        // multiplying a vec3(0,0,0) by small fractions may lead to NAN values
        if (deltaPosition.x != 0.0f || deltaPosition.y != 0.0f || deltaPosition.z != 0.0f)
        {
            position += deltaPosition
        }
        deltaPosition = Vec3(0.0f)
    }

    fun processPanWalk(vec2: Vec2) {
        val panSpeedMultiplier = (movementSpeed/200.0f)
        deltaPosition = (Vec3(front.x, 0.0f, front.z) * vec2.y * panSpeedMultiplier) + (right * -vec2.x * panSpeedMultiplier)
    }

    fun processPanFly(vec2: Vec2) {
        val panSpeedMultiplier = (movementSpeed/200.0f)
        deltaPosition = (front * vec2.y * panSpeedMultiplier) + (right * -vec2.x * panSpeedMultiplier)
    }

    fun processRotationSensor(inverseRotMat: Mat4) {
        if(!::startingMat4Inverse.isInitialized) startingMat4Inverse = inverseRotMat
        this.rotMat = startingMat4Inverse * inverseRotMat.inverse()
    }
}