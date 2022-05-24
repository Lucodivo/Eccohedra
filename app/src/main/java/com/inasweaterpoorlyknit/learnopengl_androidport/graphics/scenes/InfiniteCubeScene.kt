package com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes

import android.content.Context
import android.opengl.GLES30.*
import android.view.MotionEvent
import androidx.core.math.MathUtils.clamp
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.*
import glm_.glm
import glm_.mat4x4.Mat4
import glm_.vec2.Vec2
import glm_.vec3.Vec3
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import kotlin.math.cos
import kotlin.math.sin

// NOTE: DS means deciseconds
class InfiniteCubeScene(context: Context) : Scene(context) {

    companion object {
        private const val cubeRotationAnglePerDS = 0.3125f * RadiansPerDegree
        private val cubeRotationAxis = Vec3(1.0f, 0.3f, 0.5f)
        private const val cubeScale: Float = 1.0f
        private const val outlineTextureIndex = 2
        private const val SMOOTH_TRANSITIONS = true
        private val cubeScaleMatrix = Mat4(cubeScale)

        private const val fovY = 45.0f
        private const val zNear = 0.1f
        private const val zFar = 100.0f

        private const val generalPanScaleFactor = 0.005f
        private const val cameraForwardMax = -2.15f
        private const val cameraForwardMin = -10.0f

        private const val actionDownWindow = 0.3f

        private object uniform {
            const val diffuseTexture = "diffTexture"
            const val viewMat = "view"
            const val modelMat = "model"
            const val projectionMat = "projection"
            const val textureWidth = "texWidth"
            const val textureHeight = "texHeight"
        }
    }

    private val camera = Camera(Vec3(0.0f, 0.0f, if (sceneOrientation.isLandscape()) -3.0f else -4.0f))
    private val cameraForwardMax = if (sceneOrientation.isLandscape()) -1.5f else -2.15f
    private val cameraForwardMin = if (sceneOrientation.isLandscape()) -5.0f else -10.0f

    // GL handles
    private lateinit var textureCropProgram: Program
    private lateinit var cubeOutlineProgram: Program
    private val frameBuffers = arrayOf(FrameBuffer(), FrameBuffer())
    private var cubeVAO: Int = -1
    private var outlineTextureId: Int = -1

    private var currentFrameBufferIndex: Int = 0
    private var previousFrameBufferIndex: Int = 1

    private var lastFrameTimeDS: Double = -1.0
    private var elapsedTimeDS: Double = 0.0
    private var timeColorOffset = 0.0f
    private var previousX: Float = 0.0f
    private var previousY: Float = 0.0f
    private var actionDownTime: Double = 0.0
    private var staggeredTimerDS: Double = 0.0 // only used when SMOOTH_TRANSITIONS is set to false, for staggered captures

    private var cubePanRotationMat = Mat4(1.0f)

    override fun onSurfaceCreated(unused: GL10, config: EGLConfig) {
        // TODO: We do not use normals and could consider avoiding unnecessary computation
        cubeOutlineProgram = Program(context, R.raw.pos_norm_tex_vertex_shader, R.raw.discard_alpha_tex_fragment_shader)
        textureCropProgram = Program(context, R.raw.pos_norm_tex_vertex_shader, R.raw.crop_center_square_tex_fragment_shader)

        // setup vertex attribute objects for cube
        val cubeVAOBuffer = IntBuffer.allocate(1)
        val cubeVBOBuffer = IntBuffer.allocate(1)
        val cubeEBOBuffer = IntBuffer.allocate(1)
        initializeCubePosTexNormAttBuffers(cubeVAOBuffer, cubeVBOBuffer, cubeEBOBuffer)

        cubeVAO = cubeVAOBuffer[0]

        // Load cube outline texture and bind it
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
        cubeOutlineProgram.setUniform(uniform.diffuseTexture, outlineTextureIndex)

        lastFrameTimeDS = systemTimeInDeciseconds()
    }

    override fun onDrawFrame(unused: GL10) {
        // NOTE: OpenGL calls must be called within specified call back functions
        // Calling OpenGL functions in other functions will surely result in bugs
        val t = systemTimeInDeciseconds()
        val deltaTimeDS = (t - lastFrameTimeDS)
        lastFrameTimeDS = t
        elapsedTimeDS += deltaTimeDS

        if(SMOOTH_TRANSITIONS) {
            // constant frame changes for cube
            previousFrameBufferIndex = currentFrameBufferIndex
            currentFrameBufferIndex = if (currentFrameBufferIndex == 0) 1 else 0
        } else {
            staggeredTimerDS += deltaTimeDS
            // control when we "change frames" for the cube
            if (staggeredTimerDS > 0.5f)
            {
                staggeredTimerDS = 0.0
                previousFrameBufferIndex = currentFrameBufferIndex
                currentFrameBufferIndex = if (currentFrameBufferIndex == 0) 1 else 0
            }
        }

        // set background color
        val backgroundColor = getTimeColor(elapsedTimeDS)
        glClearColor(backgroundColor)

        // === Draw scene to off screen frame buffer ====
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[currentFrameBufferIndex].index)
        glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)

        val viewMat = camera.getViewMatrix()

        // draw cube
        // rotate with time
        val cubeModelMatrix = glm.rotate(
            cubePanRotationMat * cubeScaleMatrix,
            elapsedTimeDS.toFloat() * cubeRotationAnglePerDS,
            cubeRotationAxis
        )

        // draw cube outline
        glBindVertexArray(cubeVAO)
        cubeOutlineProgram.use()
        cubeOutlineProgram.setUniform(uniform.viewMat, viewMat)
        cubeOutlineProgram.setUniform(uniform.modelMat, cubeModelMatrix)
        glDrawElements( GL_TRIANGLES, cubePosTextNormNumVertices, GL_UNSIGNED_INT, 0 /* offset in the EBO*/)

        // draw texture within cube
        textureCropProgram.use()
        textureCropProgram.setUniform(uniform.viewMat, viewMat)
        textureCropProgram.setUniform(uniform.modelMat, cubeModelMatrix)
        textureCropProgram.setUniform(uniform.diffuseTexture, previousFrameBufferIndex)
        glDrawElements(GL_TRIANGLES, cubePosTextNormNumVertices, GL_UNSIGNED_INT, 0)

        // === Blit off screen render buffer to screen ====
        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffers[currentFrameBufferIndex].index)
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        super.onSurfaceChanged(gl, width, height)
        frameBuffers[0] = initializeFrameBuffer(windowWidth, windowHeight)
        frameBuffers[1] = initializeFrameBuffer(windowWidth, windowHeight)

        // start frame buffer with single color
        glClearColor(getTimeColor(elapsedTimeDS))

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[0].index)
        glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[1].index)
        glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)

        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, frameBuffers[0].textureBufferIndex)
        glActiveTexture(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D, frameBuffers[1].textureBufferIndex)
        glActiveTexture(GL_TEXTURE0)

        val projectionMat = glm.perspective(glm.radians(fovY), aspectRatio, zNear, zFar)

        textureCropProgram.use()
        textureCropProgram.setUniform(uniform.textureWidth, windowWidth.toFloat())
        textureCropProgram.setUniform(uniform.textureHeight, windowHeight.toFloat())
        textureCropProgram.setUniform(uniform.projectionMat, projectionMat)

        cubeOutlineProgram.use()
        cubeOutlineProgram.setUniform(uniform.projectionMat, projectionMat)
    }

    private fun getTimeColor(time: Double = systemTimeInDeciseconds()) : Vec3 {
        val t = time + timeColorOffset
        val lightR = (sin(glm.radians(t)) * 0.5f) + 0.5f
        val lightG = (cos(glm.radians(t) * 0.5f)) + 0.5f
        val lightB = (sin(glm.radians(t + 180.0f)) * 0.5f) + 0.5f
        return Vec3(lightR, lightG, lightB)
    }

    private fun pan(panVal: Vec2) {
        val maxPanY = cameraForwardMax - camera.position.z
        val minPanY = cameraForwardMin - camera.position.z
        val scaledAndClampedPanY = clamp(panVal.y * generalPanScaleFactor, minPanY, maxPanY)

        camera.moveForward(scaledAndClampedPanY)

        // TODO: Maybe the pan could also "fling"? Meaning it continues to move after the user lets go.
        cubePanRotationMat = glm.rotate(
                cubePanRotationMat,
                panVal.x * generalPanScaleFactor * 0.2f,
                Vec3(0.0f, 1.0f, 0.0f)
        )
    }

    fun action() {
        timeColorOffset += 100.0f
        if(timeColorOffset >= 1000.0f) timeColorOffset = 0.0f
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                previousX = event.x
                previousY = event.y
                actionDownTime = systemTimeInSeconds()
                return true
            }
            MotionEvent.ACTION_MOVE -> {

                val dx: Float = event.x - previousX
                val dy: Float = event.y - previousY

                pan(Vec2(dx, dy))

                previousX = event.x
                previousY = event.y
                return true
            }
            MotionEvent.ACTION_UP -> {
                if((systemTimeInSeconds() - actionDownTime) <= actionDownWindow) {
                    action()
                }
                return true
            }
            else -> {
                return super.onTouchEvent(event)
            }
        }
    }
}
