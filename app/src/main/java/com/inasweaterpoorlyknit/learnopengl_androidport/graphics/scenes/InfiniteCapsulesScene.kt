package com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes

import android.content.Context
import android.opengl.GLES20.*
import android.opengl.GLES30.glBindVertexArray
import android.view.MotionEvent
import com.inasweaterpoorlyknit.Mat3
import com.inasweaterpoorlyknit.Vec2
import com.inasweaterpoorlyknit.Vec3
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.*
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class InfiniteCapsulesScene(context: Context) : Scene(context) {

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
    private val rotationSensorHelper = RotationSensorHelper()
    private lateinit var program: Program
    private var quadVAO: Int = -1

    private var elapsedTime : Double = .0
    private var lastFrameTime: Double = -1.0
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
        program = Program(context, R.raw.uv_coord_vertex_shader, R.raw.infinite_capsules_fragment_shader)

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

        val rotationMat = rotationSensorHelper.getRotationMatrix(sceneOrientation)
        moveCameraForward(rotationMat, (deltaTime * cameraSpeed).toFloat())

        glClear(GL_COLOR_BUFFER_BIT)

        program.setUniform(uniform.rayOrigin, cameraPos)
        program.setUniform(uniform.elapsedTime, elapsedTime.toFloat())
        program.setUniform(uniform.cameraRotationMat, rotationMat)
        program.setUniform(uniform.lightPosition, lightPosition)
        if(lightAlive) {
            val deltaDistDelta = (deltaTime * lightSpeed).toFloat()
            val lightPosDelta: Vec3 = lightMoveDir * deltaDistDelta
            lightPosition = lightPosition + lightPosDelta
            lightDistanceTraveled += deltaDistDelta
            if(lightDistanceTraveled > lightMaxTravelDist) lightAlive = false
        }

        // Draw triangles from the forever-bounded quadVAO
        glDrawElements(GL_TRIANGLES, frameBufferQuadNumVertices, GL_UNSIGNED_INT, 0 /* offset in the EBO */)
    }

    override fun onAttach() {
        rotationSensorHelper.init(context)
    }

    override fun onDetach() {
        rotationSensorHelper.deinit(context)
    }

    private fun action() {
        lightAlive = true
        lightDistanceTraveled = 0f
        lightMoveDir = cameraForward
        lightPosition = cameraPos + lightMoveDir
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                actionDownTime = systemTimeInSeconds()
                return true
            }
            MotionEvent.ACTION_MOVE -> {
                if(systemTimeInSeconds() - actionDownTime > actionTimeFrame) {
                    cameraSpeed = cameraSpeedFast
                }
                return true
            }
            MotionEvent.ACTION_UP -> {
                if((systemTimeInSeconds() - actionDownTime) <= actionTimeFrame) {
                    action()
                } else {
                    cameraSpeed = cameraSpeedNormal
                }
                return true
            }
            else -> {
                return super.onTouchEvent(event)
            }
        }
    }
}