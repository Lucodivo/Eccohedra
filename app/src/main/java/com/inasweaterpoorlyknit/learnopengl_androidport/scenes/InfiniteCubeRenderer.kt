package com.inasweaterpoorlyknit.learnopengl_androidport.scenes

import android.content.Context
import javax.microedition.khronos.egl.EGLConfig

import android.opengl.GLES31.*
import android.opengl.GLSurfaceView
import android.opengl.Matrix
import com.inasweaterpoorlyknit.learnopengl_androidport.*
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.radians
import java.nio.IntBuffer
import javax.microedition.khronos.opengles.GL10
import kotlin.math.sin

const val cubeRotationAngle = 2.5f
const val currentFrameBufferIndex = 0
const val previousFrameBufferIndex = 1
const val outlineTextureIndex = 2

class InfiniteCubeRenderer(private val context: Context) : GLSurfaceView.Renderer {

    lateinit var camera: Camera
    lateinit var cubeProgram: Program
    lateinit var cubeOutlineProgram: Program
    lateinit var frameBufferProgram: Program
    lateinit var frameBuffer1: FrameBuffer
    lateinit var frameBuffer2: FrameBuffer
    private var cubeVAO: Int = -1
    private var quadVAO: Int = -1
    private var outlineTextureId: Int = -1
    private var viewportHeight: Int = -1
    private var viewportWidth: Int = -1
    private var projectionMat = FloatArray(4*4)
    private var lastFrameTime: Float = -1.0f

    override fun onSurfaceCreated(unused: GL10, config: EGLConfig) {
        camera = Camera()
        cubeProgram = Program(context, R.raw.pos_norm_tex_vertex_shader, R.raw.crop_center_square_tex_fragment_shader)
        cubeOutlineProgram = Program(context, R.raw.pos_norm_tex_vertex_shader, R.raw.discard_alpha_fragment_shader)
        frameBufferProgram = Program(context, R.raw.frame_buffer_vertex_shader, R.raw.basic_texture_fragment_shader)

        // TODO: Delete buffers when renderer completes?
        // setup vertex attribute objects
        val cubeVAOBuffer = IntBuffer.allocate(1)
        val cubeVBOBuffer = IntBuffer.allocate(1)
        val cubeEBOBuffer = IntBuffer.allocate(1)
        initializeCubePosTexNormAttBuffers(cubeVAOBuffer, cubeVBOBuffer, cubeEBOBuffer)
        val quadVAOBuffer = IntBuffer.allocate(1)
        val quadVBOBuffer = IntBuffer.allocate(1)
        val quadEBOBuffer = IntBuffer.allocate(1)
        initializeCubePosTexNormAttBuffers(quadVAOBuffer, quadVBOBuffer, quadEBOBuffer)

        cubeVAO = cubeVAOBuffer[0]
        quadVAO = quadVAOBuffer[0]

        // Set the background frame color
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f)

        outlineTextureId = loadTexture(context, R.raw.cube_outline)
        glActiveTexture(GL_TEXTURE0 + outlineTextureIndex)
        glBindTexture(GL_TEXTURE_2D, outlineTextureId)
        glActiveTexture(GL_TEXTURE0)

        // Redraw background color
        glClear(GL_COLOR_BUFFER_BIT)

        glEnable(GL_CULL_FACE)
        glCullFace(GL_BACK)
        glFrontFace(GL_CCW)

        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

        glEnable(GL_DEPTH_TEST)
        glDepthFunc(GL_LESS)

        // set constant uniforms
        cubeProgram.use()
        cubeProgram.setUniform("projection", projectionMat)

        cubeOutlineProgram.use()
        cubeOutlineProgram.setUniform("projection", projectionMat)
        cubeOutlineProgram.setUniform("diffTexture", outlineTextureIndex)

        lastFrameTime = getTime() / 2
    }

    override fun onDrawFrame(unused: GL10) {
        val t = getTime() / 2
        val deltaTime = t - lastFrameTime
        lastFrameTime = t

        // TODO: Frame changes for cube

        // set background color
        // smoother color change
        val lightR = (sin((t + 30.0f) / 3.0f) / 2.0f) + 0.5f
        val lightG = (sin((t + 60.0f) / 8.0f) / 2.0f) + 0.5f
        val lightB = (sin(t / 17.0f) / 2.0f) + 0.5f
        glClearColor(lightR, lightG, lightB, 1.0f)
        glClear(GL_COLOR_BUFFER_BIT)
    }

    override fun onSurfaceChanged(unused: GL10, width: Int, height: Int) {
        glViewport(0, 0, width, height)

        // delete render buffers if they've already been initialized
        if (::frameBuffer1.isInitialized) frameBuffer1.delete()
        if (::frameBuffer2.isInitialized) frameBuffer2.delete()

        frameBuffer1 = FrameBuffer()
        initializeFrameBuffer(frameBuffer1, width, height)
        frameBuffer2 = FrameBuffer()
        initializeFrameBuffer(frameBuffer2, width, height)

        // start frame buffer with single color
        glClearColor(0.5f, 0.0f, 0.0f, 1.0f)

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer1.index[0])
        glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT + GL_STENCIL_BUFFER_BIT)

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer2.index[0])
        glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT + GL_STENCIL_BUFFER_BIT)

        glActiveTexture(GL_TEXTURE0 + currentFrameBufferIndex)
        glBindTexture(GL_TEXTURE_2D, frameBuffer1.textureBufferIndex[0])
        glActiveTexture(GL_TEXTURE0 + previousFrameBufferIndex)
        glBindTexture(GL_TEXTURE_2D, frameBuffer2.textureBufferIndex[0])
        glActiveTexture(GL_TEXTURE0)

        cubeProgram.use()
        cubeProgram.setUniform("texWidth", width)
        cubeProgram.setUniform("texHeight", height)

        viewportWidth = width
        viewportHeight = height
        Matrix.perspectiveM(projectionMat, 0, radians(camera.zoom), width.toFloat()/height, 0.1f, 100.0f)

        // TODO: remove
        glBindFramebuffer(GL_FRAMEBUFFER, 0)
    }

    private fun getTime() : Float {
        // note: time measured in deciseconds (10^-1 seconds)
        return System.nanoTime().toFloat() / 100000000
    }
}
