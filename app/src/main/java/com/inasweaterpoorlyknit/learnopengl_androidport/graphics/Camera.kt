package com.inasweaterpoorlyknit.learnopengl_androidport.graphics

import glm_.glm
import glm_.mat4x4.Mat4
import glm_.vec2.Vec2
import glm_.vec3.Vec3

class Camera(position: Vec3 = Vec3(0.0f, 0.0f, 0.0f)) {

    companion object {
        private val worldUp = Vec3(0.0f, 1.0f, 0.0f)
    }

    var position = position
        private set

    var front = Vec3(0.0f, 0.0f, 1.0f)
        private set
    private var right = Vec3(1.0f, 0.0f, 0.0f)
    private var up: Vec3 = Vec3(0.0f, 1.0f, 0.0f)

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

        return Mat4(
            xAxis.x, yAxis.x, -zAxis.x, 0.0f,
            xAxis.y, yAxis.y, -zAxis.y, 0.0f,
            xAxis.z, yAxis.z, -zAxis.z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        )
    }

    // Calculates the front vector from the Camera's (updated) Eular Angles
    private fun updateCameraVectors() {
        // Calculate the new front vector
//        val newFront = rotMat * Vec4(0.0f, 0.0f, -1.0f, 1.0f)

//        front = glm.normalize(newFront.toVec3())
//        // Also re-calculate the right and Up vector
//        right = glm.normalize(glm.cross(front, worldUp))  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
//        up = glm.normalize(glm.cross(right, front))
    }

    private fun changePositioning()
    {
        position.plusAssign(deltaPosition)
        deltaPosition.x = 0.0f
        deltaPosition.y = 0.0f
        deltaPosition.z = 0.0f
    }

    fun moveForward(unitsForward: Float) = position.plusAssign(front * unitsForward)

    // Left handed coordinate system: X is right, Z is forward, Y is up
    // screenPan.x: move along the right vector (negated to feel like a "drag")
    // screenPan.y: +Y is a downward pan movement. Panning down moves forward. Panning up moves back. (pulling things toward you, pushing them away)
    fun processScreenPanWalk(screenPan: Vec2) {
        deltaPosition = (Vec3(front.x, 0.0f, front.z).normalize() * screenPan.y) + (right * -screenPan.x)
    }

    fun processScreenPanFly(vec2: Vec2) {
        deltaPosition = (front * vec2.y) + (right * -vec2.x)
    }
}