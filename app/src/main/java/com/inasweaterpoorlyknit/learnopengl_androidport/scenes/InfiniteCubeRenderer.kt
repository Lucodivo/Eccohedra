package com.inasweaterpoorlyknit.learnopengl_androidport.scenes

import android.content.Context
import android.opengl.GLES20.*
import android.opengl.GLES30.glBindVertexArray
import android.opengl.GLSurfaceView
import com.inasweaterpoorlyknit.learnopengl_androidport.*
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.loadTexture
import glm_.glm
import glm_.mat4x4.Mat4
import glm_.vec3.Vec3
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import kotlin.math.cos
import kotlin.math.sin
import com.inasweaterpoorlyknit.learnopengl_androidport.scenes.InfiniteCubeRenderer.GrowState.*
import glm_.vec2.Vec2


const val cubeRotationAngle = 2.5f
val cubeRotationAxis = Vec3(1.0f, 0.3f, 0.5f)
const val outlineTextureIndex = 2

const val SMOOTH_TRANSITIONS = true

class InfiniteCubeRenderer(private val context: Context) : GLSurfaceView.Renderer {

    private lateinit var camera: Camera
    private lateinit var cubeProgram: Program
    private lateinit var cubeOutlineProgram: Program
    private lateinit var frameBufferProgram: Program
    private lateinit var frameBuffers: Array<FrameBuffer>
    private var cubeVAO: Int = -1
    private var quadVAO: Int = -1
    private var outlineTextureId: Int = -1
    private var viewportHeight: Int = -1
    private var viewportWidth: Int = -1
    private var projectionMat = Mat4()
    private var lastFrameTime: Float = -1.0f

    private var currentFrameBufferIndex: Int = 0
    private var previousFrameBufferIndex: Int = 1
    private var cubeScale: Float = 1.0f
    private var growScaleMax: Float = 1.5f

    override fun onSurfaceCreated(unused: GL10, config: EGLConfig) {
        camera = Camera()
        cubeProgram = Program(context, R.raw.pos_norm_tex_vertex_shader, R.raw.crop_center_square_tex_fragment_shader)
        cubeOutlineProgram = Program(context, R.raw.pos_norm_tex_vertex_shader, R.raw.discard_alpha_fragment_shader)
        frameBufferProgram = Program(context, R.raw.frame_buffer_vertex_shader, R.raw.basic_texture_fragment_shader)

        // setup vertex attribute objects
        val cubeVAOBuffer = IntBuffer.allocate(1)
        val cubeVBOBuffer = IntBuffer.allocate(1)
        val cubeEBOBuffer = IntBuffer.allocate(1)
        initializeCubePosTexNormAttBuffers(cubeVAOBuffer, cubeVBOBuffer, cubeEBOBuffer)
        val quadVAOBuffer = IntBuffer.allocate(1)
        val quadVBOBuffer = IntBuffer.allocate(1)
        val quadEBOBuffer = IntBuffer.allocate(1)
        initializeFrameBufferQuadVertexAttBuffers(quadVAOBuffer, quadVBOBuffer, quadEBOBuffer)

        cubeVAO = cubeVAOBuffer[0]
        quadVAO = quadVAOBuffer[0]

        outlineTextureId = loadTexture(context, R.raw.cube_outline)
        glActiveTexture(GL_TEXTURE0 + outlineTextureIndex)
        glBindTexture(GL_TEXTURE_2D, outlineTextureId)
        glActiveTexture(GL_TEXTURE0)

        glEnable(GL_CULL_FACE)
        glCullFace(GL_BACK)
        glFrontFace(GL_CCW)

        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

        glEnable(GL_DEPTH_TEST)
        glDepthFunc(GL_LESS)

        cubeOutlineProgram.use()
        cubeOutlineProgram.setUniform("diffTexture", outlineTextureIndex)

        lastFrameTime = getDeciseconds()
    }

    private var elapsedTime = 0.0f
    private var staggeredTimer = 0.0f;
    override fun onDrawFrame(unused: GL10) {
        val t = getDeciseconds()
        val deltaDeciseconds = (t - lastFrameTime)
        lastFrameTime = t
        elapsedTime += deltaDeciseconds

        if(SMOOTH_TRANSITIONS) {
            // constant frame changes for cube
            previousFrameBufferIndex = currentFrameBufferIndex;
            currentFrameBufferIndex = if (currentFrameBufferIndex == 0) 1 else 0
        } else {
            staggeredTimer += deltaDeciseconds
            // control when we "change frames" for the cube
            if (staggeredTimer > 5.0f)
            {
                staggeredTimer = 0.0f
                previousFrameBufferIndex = currentFrameBufferIndex
                currentFrameBufferIndex = if (currentFrameBufferIndex == 0) 1 else 0
            }
        }

        // set background color
        val lightR = (sin(glm.radians(elapsedTime)) / 2.0f) + 0.5f
        val lightG = (cos(glm.radians(elapsedTime) / 2.0f)) + 0.5f
        val lightB = (sin(glm.radians(elapsedTime + 180.0f)) / 2.0f) + 0.5f
        glClearColor(lightR, lightG, lightB, 1.0f)

        // bind default frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[currentFrameBufferIndex].index[0])

        glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)

        val viewMat = camera.getViewMatrix(deltaDeciseconds)

        // draw cube
        // rotate with time
        if(growState != None) {

            val deltaScale = 0.004f/deltaDeciseconds
            if(growState == Growing) { // grow
                cubeScale += deltaScale
                if(cubeScale >= growScaleMax) growState = Shrinking
            } else { // shrink
                cubeScale -= deltaScale
            }

            if(cubeScale <= 1.0f) {
                growState = None
                cubeScale = 1.0f
            }
        }

        var cubeModelMatrix = Mat4()
        cubeModelMatrix = glm.scale(
            cubeModelMatrix,
            Vec3(cubeScale)
        )
        cubeModelMatrix = glm.rotate(
            cubeModelMatrix,
            (elapsedTime/8) * glm.radians(cubeRotationAngle),
            cubeRotationAxis
        )

        // draw cube outline
        cubeOutlineProgram.use()
        glBindVertexArray(cubeVAO)
        cubeOutlineProgram.setUniform("view", viewMat)
        cubeOutlineProgram.setUniform("model", cubeModelMatrix)
        glDrawElements(
            GL_TRIANGLES,
            cubePosTextNormNumElements * 3, // number of elements to draw (3 vertices per triangle * 2 triangles per face * 6 faces)
            GL_UNSIGNED_INT,
            0
        )

        // draw texture within cube
        cubeProgram.use()
        glBindVertexArray(cubeVAO)
        cubeProgram.setUniform("view", viewMat)
        cubeProgram.setUniform("model", cubeModelMatrix)
        cubeProgram.setUniform("diffTexture", previousFrameBufferIndex)
        glDrawElements(
            GL_TRIANGLES,
            cubePosTextNormNumElements * 3, // number of elements to draw (3 vertices per triangle * 2 triangles per face * 6 faces)
            GL_UNSIGNED_INT,
            0
        )

        // draw scene to quad
        glBindFramebuffer(GL_FRAMEBUFFER, 0)
        glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)

        frameBufferProgram.use()
        glBindVertexArray(quadVAO)
        frameBufferProgram.setUniform("screenTexture", currentFrameBufferIndex)
        glDrawElements(GL_TRIANGLES, // drawing mode
            6, // number of elements to draw (3 vertices per triangle * 2 triangles per quad)
            GL_UNSIGNED_INT, // type of the indices
            0) // offset in the EBO
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        viewportWidth = width
        viewportHeight = height

        glViewport(0, 0, viewportWidth, viewportHeight)

        frameBuffers = arrayOf(FrameBuffer(), FrameBuffer())

        initializeFrameBuffer(frameBuffers[0], viewportWidth, viewportHeight)
        initializeFrameBuffer(frameBuffers[1], viewportWidth, viewportHeight)

        // start frame buffer with single color
        val timeColor = getTimeColor()
        glClearColor(timeColor.r, timeColor.g, timeColor.b, 1.0f)

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[0].index[0])
        glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[1].index[0])
        glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)

        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, frameBuffers[0].textureBufferIndex[0])
        glActiveTexture(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D, frameBuffers[1].textureBufferIndex[0])
        glActiveTexture(GL_TEXTURE0)

        projectionMat = glm.perspective(glm.radians(camera.zoom), viewportWidth.toFloat()/viewportHeight, 0.1f, 100.0f)

        cubeProgram.use()
        cubeProgram.setUniform("texWidth", viewportWidth.toFloat())
        cubeProgram.setUniform("texHeight", viewportHeight.toFloat())
        cubeProgram.setUniform("projection", projectionMat)

        cubeOutlineProgram.use()
        cubeOutlineProgram.setUniform("projection", projectionMat)
    }


    var growState: GrowState = None
    fun touch() {
        if(growState == None) {
            growState = Growing
        }
    }

    private fun getDeciseconds() : Float {
        // note: time measured in deciseconds (10^-1 seconds)
        return System.nanoTime().toFloat() / 100000000
    }

    private fun getTimeColor() : Vec3 {
        val t = getDeciseconds()
        val lightR = (sin(glm.radians(t)) / 2.0f) + 0.5f
        val lightG = (cos(glm.radians(t) / 2.0f)) + 0.5f
        val lightB = (sin(glm.radians(t + 180.0f)) / 2.0f) + 0.5f
        return Vec3(lightR, lightG, lightB)
    }

    fun pan(vec2: Vec2) {
        camera.processPan(vec2)
    }

    enum class GrowState {
        Growing,
        Shrinking,
        None
    }
}
