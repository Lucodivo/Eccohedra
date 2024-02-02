package com.inasweaterpoorlyknit.learnopengl_androidport.graphics

import com.inasweaterpoorlyknit.Mat4
import com.inasweaterpoorlyknit.Vec3

// Note: Currently the camera faces (+Z) and can move forward or back. Anything else hasn't been needed.
class Camera(position: Vec3 = Vec3(0.0f, 0.0f, 0.0f)) {

    var position = position
        private set

    var front = Vec3(0.0f, 0.0f, 1.0f)
        private set
    private var right = Vec3(1.0f, 0.0f, 0.0f)
    private var up: Vec3 = Vec3(0.0f, 1.0f, 0.0f)

    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    fun getViewMatrix() : Mat4
    {
        val target = position + front

        // Calculate cameraDirection
        val zAxis = (position - target).normalized
        // Get positive right axis vector
        val xAxis = up.normalized.cross(zAxis).normalized
        // Calculate camera up vector
        val yAxis = zAxis.cross(xAxis)

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

    fun moveForward(unitsForward: Float) {
        position += front * unitsForward
    }
}