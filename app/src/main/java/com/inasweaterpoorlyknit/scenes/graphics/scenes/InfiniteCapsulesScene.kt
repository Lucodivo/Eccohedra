package com.inasweaterpoorlyknit.scenes.graphics.scenes

import android.content.res.Resources
import android.opengl.GLES30.glBindVertexArray
import android.opengl.GLES32.GL_COLOR_BUFFER_BIT
import android.opengl.GLES32.GL_TRIANGLES
import android.opengl.GLES32.GL_UNSIGNED_INT
import android.opengl.GLES32.glClear
import android.opengl.GLES32.glDrawElements
import android.view.MotionEvent
import com.inasweaterpoorlyknit.noopmath.Mat3
import com.inasweaterpoorlyknit.noopmath.Vec3
import com.inasweaterpoorlyknit.noopmath.clamp
import com.inasweaterpoorlyknit.noopmath.mod
import com.inasweaterpoorlyknit.scenes.R
import com.inasweaterpoorlyknit.scenes.graphics.*
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class InfiniteCapsulesScene(private val rotationSensorHelper: RotationSensorHelper, private val resources: Resources, private val startingOrientation: Orientation) : Scene() {

    companion object {
        // TODO: Consider linking capsule/container dimen to uniform?
        const val repeatedContainerDimen = 6f // NOTE: This value MUST match two times the boxDimens constant in the shader!!!

        private object uniform {
            const val lightColor = "lightColor"
            const val lightPosition = "lightPos"
            const val viewPortResolution = "viewPortResolution"
            const val rayOrigin = "rayOrigin"
            const val elapsedTime = "elapsedTime"
            const val cameraRotationMat = "cameraRotationMat"
        }

        private val defaultCameraForward = Vec3(0f, 0f, 1f)
    }

    private var cameraPos = Vec3(0f, 1f, 0f)
    private var cameraForward = defaultCameraForward
    private lateinit var program: Program
    private var quadVAO: Int = -1

    private var elapsedTime : Double = 0.0
    private var lastFrameTime: Double = 0.0
    private var firstFrameTime: Double = -1.0

    private var lightAlive = false
    private var lightPosition = Vec3(0f, 0f, 100f)
    private val lightColor = Vec3(.5f, .5f, .5f)
    private var lightMoveDir = Vec3(0f, 0f, 0f)
    private val lightMaxTravelDist = 100f
    private var lightDistanceTraveled = 0f
    private val lightSpeed = 2f

    private val cameraSpeedNormal = .5f
    private val cameraSpeedFast = 2f
    private var cameraSpeed = cameraSpeedNormal

    private var actionDownTime: Double = .0
    private val actionTimeFrame = .1f

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        program = Program(resources, R.raw.uv_coord_vertex_shader, R.raw.infinite_capsules_fragment_shader)

        // setup vertex attributes for quad
        val quadVAOBuffer = IntBuffer.allocate(1)
        val quadVBOBuffer = IntBuffer.allocate(1)
        val quadEBOBuffer = IntBuffer.allocate(1)
        initializeFrameBufferQuadVertexAttBuffers(quadVAOBuffer, quadVBOBuffer, quadEBOBuffer)
        quadVAO = quadVAOBuffer[0]

        firstFrameTime = systemTimeInDeciseconds()
        glClearColor(Vec3(1f, 0f, 0f))

        program.use()
        glBindVertexArray(quadVAO)
        program.setUniform(uniform.lightColor, lightColor)
        program.setUniform(uniform.lightPosition, lightPosition)
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        super.onSurfaceChanged(gl, width, height)

        program.use()
        program.setUniform(uniform.viewPortResolution, width.toFloat(), height.toFloat())
    }

    private fun moveCameraForward(rotationMat: Mat3, units: Float) {
        cameraForward = rotationMat * defaultCameraForward

        cameraPos += (cameraForward * units)

        // prevent floating point values from growing to unreasonable values
        val originalCameraPos = cameraPos
        cameraPos %= repeatedContainerDimen
        lightPosition += cameraPos - originalCameraPos
    }

    override fun onDrawFrame(gl: GL10?) {
        // NOTE: OpenGL calls must be called within specified call back functions
        // Calling OpenGL functions in other functions will surely result in bugs
        elapsedTime = systemTimeInDeciseconds() - firstFrameTime
        val deltaTime = elapsedTime - lastFrameTime
        lastFrameTime = elapsedTime

        var rotationMat = rotationSensorHelper.getRotationMatrix(startingOrientation)
        moveCameraForward(rotationMat, (deltaTime * cameraSpeed).toFloat())

        if(sdScene(cameraPos, rotationMat) <= 0.0f) {
            resetScene()
            rotationMat = Mat3(1.0f)
        }

        glClear(GL_COLOR_BUFFER_BIT)

        program.setUniform(uniform.rayOrigin, cameraPos)
        program.setUniform(uniform.elapsedTime, elapsedTime.toFloat())
        program.setUniform(uniform.cameraRotationMat, rotationMat)
        program.setUniform(uniform.lightPosition, lightPosition)
        if(lightAlive) {
            val deltaDistDelta = (deltaTime * lightSpeed).toFloat()
            val lightPosDelta: Vec3 = lightMoveDir * deltaDistDelta
            lightPosition += lightPosDelta
            lightDistanceTraveled += deltaDistDelta
            if(lightDistanceTraveled > lightMaxTravelDist) lightAlive = false
        }

        // Draw triangles from the forever-bounded quadVAO
        glDrawElements(GL_TRIANGLES, frameBufferQuadNumVertices, GL_UNSIGNED_INT, 0 /* offset in the EBO */)
    }

    private fun resetScene() {
        cameraPos = Vec3(0f, 1f, 0f)
        cameraForward = defaultCameraForward

        elapsedTime = 0.0
        lastFrameTime = 0.0
        firstFrameTime = systemTimeInDeciseconds()

        lightAlive = false
        lightPosition = Vec3(0f, 0f, 100f)
        lightMoveDir = Vec3(0f, 0f, 0f)
        lightDistanceTraveled = 0f

        rotationSensorHelper.reset()
    }
    private val capsuleLineWidth = 3.0f
    private val capsuleLineHalfWidth = capsuleLineWidth * 0.5f

    private fun sdScene(pos: Vec3, rotationMat: Mat3): Float {
        var capsuleCenterPosA = Vec3(-capsuleLineHalfWidth, 0.0f, 0.0f)
        var capsuleCenterPosB = Vec3(capsuleLineHalfWidth, 0.0f, 0.0f)
        val capsuleContainerDimens = 6.0f
        val offset = capsuleContainerDimens * 0.5f // this value also

        capsuleCenterPosA = rotationMat * capsuleCenterPosA
        capsuleCenterPosB = rotationMat * capsuleCenterPosB
        capsuleCenterPosA += Vec3(offset)
        capsuleCenterPosB += Vec3(offset)

        val posCapsuleContainer: Vec3 = mod(pos, Vec3(capsuleContainerDimens))
        return sdCapsule(posCapsuleContainer, capsuleCenterPosA, capsuleCenterPosB, 1.0f)
    }

    private fun sdCapsule(pos: Vec3, posA: Vec3, posB: Vec3, radius: Float): Float {
        val oneOverCapsuleLineWidth = 1.0f / capsuleLineWidth

        // Line segments from point A to B (line segment of capsule)
        val aToB: Vec3 = posB - posA
        // Line from point A to ray's position
        val aToRayPos: Vec3 = pos - posA

        // find the projection of the line from A to the ray's position onto the capsule's line segment
        val abCosTheta: Float = aToB.dot(aToRayPos)
        // float magnitudeAToB = capsuleLineWidth
        val projectionAToRayOnAToB: Float = abCosTheta * oneOverCapsuleLineWidth // = abCosTheta / magnitudeAToB

        // Use the projection to walk down the capsule's line segment and find the closest point
        val closestPoint: Vec3 = posA + aToB * clamp(projectionAToRayOnAToB * oneOverCapsuleLineWidth, 0.0f, 1.0f)
        return (pos - closestPoint).len - radius
    }

    override fun onAttach() {
        rotationSensorHelper.init()
    }

    override fun onDetach() {
        rotationSensorHelper.deinit()
    }

    private fun action() {
        lightAlive = true
        lightDistanceTraveled = 0f
        lightMoveDir = cameraForward
        lightPosition = cameraPos + lightMoveDir
    }

    override fun onTouchEvent(event: MotionEvent){
        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                actionDownTime = systemTimeInSeconds()
            }
            MotionEvent.ACTION_MOVE -> {
                if(systemTimeInSeconds() - actionDownTime > actionTimeFrame) {
                    cameraSpeed = cameraSpeedFast
                }
            }
            MotionEvent.ACTION_UP -> {
                if((systemTimeInSeconds() - actionDownTime) <= actionTimeFrame) {
                    action()
                } else {
                    cameraSpeed = cameraSpeedNormal
                }
            }
            else -> {}
        }
    }
}