package com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes

import android.content.Context
import android.content.SharedPreferences
import android.opengl.GLES32.*
import android.view.MotionEvent
import com.inasweaterpoorlyknit.Vec2
import com.inasweaterpoorlyknit.Vec3
import com.inasweaterpoorlyknit.learnopengl_androidport.*
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.*
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

data class Resolution (
    val width: Int,
    val height: Int
)

class MengerPrisonScene(context: Context) : Scene(context), SharedPreferences.OnSharedPreferenceChangeListener {

    companion object {
        // TODO: Consider linking box/container dimen to uniform?
        const val repeatedContainerDimen = 40.0f // NOTE: This value MUST be equal to the capsuleContainerDimens in the fragment shader!!

        private object uniform {
            const val iterations = "iterations"
            const val viewPortResolution = "viewPortResolution"
            const val rayOrigin = "rayOrigin"
            const val cameraRotationMat = "cameraRotationMat"
        }

        val resolutionFactorOptions = arrayOf(
            1.0f/32.0f,
            1.0f/16.0f,
            1.0f/8.0f,
            1.0f/4.0f,
            1.0f/2.0f,
            1.0f,
        )
        const val defaultResolutionIndex = 3

        private const val maxIterations = 5
        private val defaultCameraForward = Vec3(0.0f, 0.0f, 1.0f)
        private const val cameraNormalSpeed = 0.5f
        private const val cameraFastSpeed = 1.5f
    }

    private lateinit var mengerPrisonProgram: Program
    private var quadVAO: Int = -1
    private var offscreenFramebuffer = FrameBuffer()

    private lateinit var resolutions: Array<Resolution>
    private var currentResolutionIndex: Int
    private var prevFrameResolutionIndex: Int // informs us of a resolution change within onDrawFrame()
    private val resolution get() = resolutions[currentResolutionIndex]

    private var cameraPos = Vec3(0.0f, 0.0f, 0.0f)
    private var cameraForward = defaultCameraForward

    private var elapsedTime : Double = 0.0
    private var lastFrameTime: Double = -1.0
    private var firstFrameTime: Double = -1.0

    private var actionDown = false

    private val rotationSensorHelper = RotationSensorHelper()

    init {
        val sharedPreferences = context.getSharedPreferences()
        currentResolutionIndex = resolveResolutionIndex(sharedPreferences)
        prevFrameResolutionIndex = currentResolutionIndex
        sharedPreferences.registerOnSharedPreferenceChangeListener(this)
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        mengerPrisonProgram = Program(context, R.raw.uv_coord_vertex_shader, R.raw.menger_prison_fragment_shader)

        // setup vertex attributes for quad
        val quadVAOBuffer = IntBuffer.allocate(1)
        val quadVBOBuffer = IntBuffer.allocate(1)
        val quadEBOBuffer = IntBuffer.allocate(1)
        initializeFrameBufferQuadVertexAttBuffers(quadVAOBuffer, quadVBOBuffer, quadEBOBuffer)
        quadVAO = quadVAOBuffer[0]
        glBindVertexArray(quadVAO)

        firstFrameTime = systemTimeInDeciseconds()
        glClearColor(Vec3(0.5f, 0.0f, 0.0f))

        mengerPrisonProgram.use()
        mengerPrisonProgram.setUniform(uniform.iterations, maxIterations)
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        super.onSurfaceChanged(gl, width, height)

        // surface resolution has changed, adjust accordingly
        initResolutions(width, height)
        offscreenFramebuffer.delete()
        offscreenFramebuffer = initializeFrameBuffer(resolution.width, resolution.height)

        mengerPrisonProgram.use()
        mengerPrisonProgram.setUniform(uniform.viewPortResolution, resolution.width.toFloat(), resolution.height.toFloat())
    }

    private fun initResolutions(width: Int, height: Int) {
        resolutions = Array(resolutionFactorOptions.size) { i ->
            val resFactor = resolutionFactorOptions[i]
            Resolution((width * resFactor).toInt(), (height * resFactor).toInt())
        }
    }

    override fun onDrawFrame(gl: GL10?) {
        // NOTE: OpenGL calls must be called within specified call back functions
        // Calling OpenGL functions in other functions will surely result in bugs
        elapsedTime = systemTimeInDeciseconds() - firstFrameTime
        val deltaTime = elapsedTime - lastFrameTime
        lastFrameTime = elapsedTime

        // Update scene
        val rotationMat = rotationSensorHelper.getRotationMatrix(sceneOrientation)
        cameraForward = rotationMat * defaultCameraForward
        val cameraSpeed = if(actionDown) cameraFastSpeed else cameraNormalSpeed
        cameraPos += cameraForward * cameraSpeed * deltaTime.toFloat()
        // prevent floating point values from growing to unreasonable values
        cameraPos %= repeatedContainerDimen

        if(prevFrameResolutionIndex != currentResolutionIndex) {
            // new resolution index changed by user, adjust accordingly
            offscreenFramebuffer.delete()
            offscreenFramebuffer = initializeFrameBuffer(resolution.width, resolution.height)
            prevFrameResolutionIndex = currentResolutionIndex
            mengerPrisonProgram.setUniform(uniform.viewPortResolution, resolution.width.toFloat(), resolution.height.toFloat())
        }

        // draw to offscreen framebuffer
        glViewport(0, 0, resolution.width, resolution.height)
        glBindFramebuffer(GL_FRAMEBUFFER, offscreenFramebuffer.index)
        glClear(GL_COLOR_BUFFER_BIT)

        mengerPrisonProgram.use()
        mengerPrisonProgram.setUniform(uniform.rayOrigin, cameraPos)
        mengerPrisonProgram.setUniform(uniform.cameraRotationMat, rotationMat)
        glDrawElements(GL_TRIANGLES, frameBufferQuadNumVertices, GL_UNSIGNED_INT, 0 /* offset in the EBO */)

        // blit rendered image onto onscreen buffer, scaling with "blocky"/"point" sampling where needed
        glViewport(0, 0, windowWidth, windowHeight)
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0)
        glBindFramebuffer(GL_READ_FRAMEBUFFER, offscreenFramebuffer.index)
        glBlitFramebuffer(0, 0, resolution.width, resolution.height, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST)
    }

    override fun onAttach() {
        rotationSensorHelper.init(context)
    }

    override fun onDetach() {
        rotationSensorHelper.init(context)
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        return when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                actionDown = true
                true
            }
            MotionEvent.ACTION_UP -> {
                actionDown = false
                true
            }
            else -> {
                super.onTouchEvent(event)
            }
        }
    }

    override fun onSharedPreferenceChanged(sharedPreferences: SharedPreferences, key: String?) {
        // NOTE: This could become costly if SharedPreferences are being edited all the time
        if(key == SharedPrefKeys.mengerPrisonResolutionIndex) {
            currentResolutionIndex = resolveResolutionIndex(sharedPreferences)
        }
    }

    // Ensuring index is never out of bounds
    private fun resolveResolutionIndex(sharedPreferences: SharedPreferences): Int {
        val newResolutionIndex = sharedPreferences.getMengerSpongeResolutionIndex()
        if(newResolutionIndex < 0 || newResolutionIndex >= resolutionFactorOptions.size) {
            sharedPreferences.setMengerSpongeResolutionIndex(defaultResolutionIndex)
            return defaultResolutionIndex
        }
        return newResolutionIndex
    }
}