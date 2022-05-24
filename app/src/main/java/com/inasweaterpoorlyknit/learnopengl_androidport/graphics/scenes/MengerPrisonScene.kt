package com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes

import android.content.Context
import android.content.SharedPreferences
import android.opengl.GLES20.GL_UNSIGNED_INT
import android.opengl.GLES20.glClear
import android.opengl.GLES30.*
import android.util.Log
import android.util.Log.DEBUG
import android.view.MotionEvent
import com.inasweaterpoorlyknit.learnopengl_androidport.*
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.*
import glm_.vec2.Vec2
import glm_.vec3.Vec3
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10


data class Resolution (
    val width: Int,
    val height: Int
)

class MengerPrisonScene(context: Context) : Scene(context), SharedPreferences.OnSharedPreferenceChangeListener {

    companion object {
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

    private lateinit var resolutions: Array<Resolution>

    private val cameraPos = Vec3(0.0f, 0.0f, 0.0f)
    private var cameraForward = defaultCameraForward

    private var elapsedTime : Double = 0.0
    private var lastFrameTime: Double = -1.0
    private var firstFrameTime: Double = -1.0

    private var actionDown = false

    private val rotationSensorHelper = RotationSensorHelper()

    private var currentResolutionIndex: Int
    private var prevFrameResolutionIndex: Int // informs us of a resolution change within onDrawFrame()
    private val resolution get() = resolutions[currentResolutionIndex]

    private var frameBufferIndex_IntArray = IntArray(1)
    private val frameBufferIndex get() = frameBufferIndex_IntArray[0]
    private var frameBufferRenderBufferIndex_IntArray = IntArray(1)
    private val frameBufferRenderBufferIndex get() = frameBufferRenderBufferIndex_IntArray[0]
    private var frameBufferTexture_IntArray = IntArray(1)
    private val frameBufferTexture get() = frameBufferTexture_IntArray[0]

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
        initResolutions(width, height)
        initializeFrameBuffer(resolution.width, resolution.height)

        mengerPrisonProgram.use()
        mengerPrisonProgram.setUniform(uniform.viewPortResolution, Vec2(resolution.width, resolution.height))
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
        if(actionDown) {
            cameraPos.plusAssign(cameraForward * (deltaTime * cameraFastSpeed).toFloat())
        } else {
            cameraPos.plusAssign(cameraForward * (deltaTime * cameraNormalSpeed).toFloat())
        }

        if(prevFrameResolutionIndex != currentResolutionIndex) {
            initializeFrameBuffer(resolution.width, resolution.height)
            prevFrameResolutionIndex = currentResolutionIndex
            mengerPrisonProgram.setUniform(uniform.viewPortResolution, Vec2(resolution.width, resolution.height))
        }

        glViewport(0, 0, resolution.width, resolution.height)
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIndex)

        glClear(GL_COLOR_BUFFER_BIT)

        mengerPrisonProgram.use()
        mengerPrisonProgram.setUniform(uniform.rayOrigin, cameraPos)
        mengerPrisonProgram.setUniform(uniform.cameraRotationMat, rotationMat)
        glDrawElements(GL_TRIANGLES, // drawing mode
            6, // number of elements to draw (3 vertices per triangle * 2 triangles per quad)
            GL_UNSIGNED_INT, // type of the indices
            0) // offset in the EBO

        glViewport(0, 0, windowWidth, windowHeight)
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferIndex)
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

    override fun onSharedPreferenceChanged(sharedPreferences: SharedPreferences, key: String) {
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

    private fun initializeFrameBuffer(width: Int, height: Int) {

        glDeleteFramebuffers(1, frameBufferIndex_IntArray, 0)
        glDeleteRenderbuffers(1, frameBufferRenderBufferIndex_IntArray, 0)
        glDeleteTextures(1, frameBufferTexture_IntArray, 0)

        // creating frame buffer
        glGenFramebuffers(1, frameBufferIndex_IntArray, 0)
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIndex)

        // creating frame buffer texture
        glGenTextures(1, frameBufferTexture_IntArray, 0)
        glBindTexture(GL_TEXTURE_2D, frameBufferTexture)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, null)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glBindTexture(GL_TEXTURE_2D, 0) // unbind

        // attach texture w/ color to frame buffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, // frame buffer we're targeting (draw, read, or both)
            GL_COLOR_ATTACHMENT0, // type of attachment
            GL_TEXTURE_2D, // type of texture
            frameBufferTexture, // texture
            0) // mipmap level

        // creating render buffer to be depth/stencil buffer
        glGenRenderbuffers(1, frameBufferRenderBufferIndex_IntArray, 0)
        glBindRenderbuffer(GL_RENDERBUFFER, frameBufferRenderBufferIndex)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height)
        glBindRenderbuffer(GL_RENDERBUFFER, 0) // unbind
        // attach render buffer w/ depth & stencil to frame buffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, // frame buffer target
            GL_DEPTH_STENCIL_ATTACHMENT, // attachment point of frame buffer
            GL_RENDERBUFFER, // render buffer target
            frameBufferRenderBufferIndex)  // render buffer

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            Log.println(DEBUG, MengerPrisonScene::class.java.canonicalName,"ERROR::FRAMEBUFFER:: Framebuffer is not complete!")
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0)
    }
}