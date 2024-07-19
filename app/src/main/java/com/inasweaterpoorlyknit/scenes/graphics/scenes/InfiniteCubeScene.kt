package com.inasweaterpoorlyknit.scenes.graphics.scenes

import android.content.Context
import android.content.res.Resources
import android.opengl.GLES30.*
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import androidx.core.math.MathUtils.clamp
import com.inasweaterpoorlyknit.noopmath.Mat3
import com.inasweaterpoorlyknit.noopmath.Mat4
import com.inasweaterpoorlyknit.noopmath.Vec2
import com.inasweaterpoorlyknit.noopmath.Vec3
import com.inasweaterpoorlyknit.noopmath.degToRad
import com.inasweaterpoorlyknit.scenes.R
import com.inasweaterpoorlyknit.scenes.graphics.*
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import kotlin.math.abs
import kotlin.math.cos
import kotlin.math.sin

// NOTE: DS means deciseconds
class InfiniteCubeScene(context: Context, private val resources: Resources, startingOrientation: Orientation) : Scene() {

    companion object {
        private const val cubeRotationAnglePerDS = .3125f * RadiansPerDegree
        private val cubeAutoRotationAxis = Vec3(1f, .3f, .5f)
        private val cubePanRotationAxis = Vec3(0f, 1f, 0f)
        private const val cubeScale: Float = 1f
        private const val outlineTextureIndex = 2
        private const val SMOOTH_TRANSITIONS = true
        private val cubeScaleMatrix = Mat3(cubeScale)

        private const val fovY = 45.0
        private const val zNear = .1
        private const val zFar = 100.0

        private const val cameraForwardPanScaleFactor = .002
        private const val cameraForwardPinchScaleFactor = .002
        private const val panRotateScaleFactor = .001

        private const val rotationDragPercent = .98f
        private const val rotationVelocityMax = 10.0
        private const val rotationStopVelocity = .05

        private const val cameraForwardDragPercent = .92f
        private const val cameraForwardVelocityMax = 10f
        private const val cameraForwardStopVelocity = .05f

        private object uniform {
            const val diffuseTexture = "diffTexture"
            const val viewMat = "view"
            const val modelMat = "model"
            const val projectionMat = "projection"
            const val textureWidth = "texWidth"
            const val textureHeight = "texHeight"
        }
    }

    private val camera = Camera(Vec3(0f, 0f, if (startingOrientation.isLandscape()) -3f else -4f))
    private val cameraForwardMax = if (startingOrientation.isLandscape()) -1.5f else -2.15f
    private val cameraForwardMin = if (startingOrientation.isLandscape()) -5f else -10f

    // GL handles
    private lateinit var textureCropProgram: Program
    private lateinit var cubeOutlineProgram: Program
    private val frameBuffers = arrayOf(FrameBuffer(), FrameBuffer())
    private var cubeVAO: Int = -1
    private var outlineTextureId: Int = -1

    private var currentFrameBufferIndex: Int = 0
    private var previousFrameBufferIndex: Int = 1

    private var lastFrameTimeDS: Double = -1.0
    private var elapsedTimeDS: Double = .0
    private var timeColorOffset = 0f
    private var staggeredTimerDS: Double = .0 // only used when SMOOTH_TRANSITIONS is set to false, for staggered captures

    private var cubePanRotationMat = Mat4(1f)

    private var rotationVelocity = 0f
    private var cameraForwardVelocity = 0.0

    private var pinchInProgress = false
    private var postPinchZoom_panFlushRequired = false
    private var prevScaleGestureFocus: Vec2 = Vec2(0f, 0f)

    private val gestureDetector = GestureDetector(context, object : GestureDetector.SimpleOnGestureListener() {
        var firstEventSinceDown = true

        override fun onDown(e: MotionEvent): Boolean { // must have or SimpleOnGestureListener doesn't work 🤷‍♀️
            firstEventSinceDown = true
            return true
        }

        override fun onSingleTapUp(e: MotionEvent): Boolean {
            jumpTimeColorOffset()
            return true
        }

        override fun onScroll(e1: MotionEvent?, e2: MotionEvent, distanceX: Float, distanceY: Float): Boolean {
            if(firstEventSinceDown) { // first event can contain distances that are essentially garbage
                firstEventSinceDown = false
            } else {
                val absDistX = abs(distanceX)
                val distXIsBigger = absDistX > abs(distanceY)
                if(distXIsBigger && absDistX > .01) {
                    rotationVelocity = 0f
                    rotateCube(-distanceX * panRotateScaleFactor)
                } else {
                    cameraForwardVelocity = 0.0
                    moveCameraForward(-distanceY * cameraForwardPanScaleFactor)
                }
            }
            return true
        }

        override fun onFling(e1: MotionEvent?, e2: MotionEvent, velocityX: Float, velocityY: Float): Boolean {

            val rotVel = (velocityX / windowWidth)
            rotationVelocity = clamp(rotVel, -cameraForwardVelocityMax, cameraForwardVelocityMax)

            val cameraForwardVel = velocityY.toDouble() / windowWidth
            cameraForwardVelocity = clamp(cameraForwardVel, -rotationVelocityMax, rotationVelocityMax)

            return true
        }
    })

    private val scaleGestureDetector = ScaleGestureDetector(context, object : ScaleGestureDetector.OnScaleGestureListener {
        override fun onScale(detector: ScaleGestureDetector): Boolean {
            // zoom
            val spanDelta = detector.currentSpan - detector.previousSpan
            moveCameraForward(spanDelta * cameraForwardPinchScaleFactor)

            // pan
            val dx: Double = detector.focusX.toDouble() - prevScaleGestureFocus.x.toDouble()
            prevScaleGestureFocus = Vec2(detector.focusX, detector.focusY)
            rotateCube(dx * panRotateScaleFactor)

            return true
        }

        override fun onScaleBegin(detector: ScaleGestureDetector): Boolean {
            prevScaleGestureFocus = Vec2(detector.focusX, detector.focusY)
            pinchInProgress = true
            return true
        }

        override fun onScaleEnd(detector: ScaleGestureDetector) {
            pinchInProgress = false
            postPinchZoom_panFlushRequired = true
        }
    })

    init {
        scaleGestureDetector.isQuickScaleEnabled = false
    }

    override fun onSurfaceCreated(unused: GL10, config: EGLConfig) {
        // TODO: We do not use normals and could consider avoiding unnecessary computation
        cubeOutlineProgram = Program(resources, R.raw.pos_norm_tex_vertex_shader, R.raw.discard_alpha_tex_fragment_shader)
        textureCropProgram = Program(resources, R.raw.pos_norm_tex_vertex_shader, R.raw.crop_center_square_tex_fragment_shader)

        // setup vertex attribute objects for cube
        val cubeVAOBuffer = IntBuffer.allocate(1)
        val cubeVBOBuffer = IntBuffer.allocate(1)
        val cubeEBOBuffer = IntBuffer.allocate(1)
        initializeCubePosTexNormAttBuffers(cubeVAOBuffer, cubeVBOBuffer, cubeEBOBuffer)

        cubeVAO = cubeVAOBuffer[0]

        // Load cube outline texture and bind it
        outlineTextureId = loadTexture(resources, R.raw.cube_outline)
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
            if (staggeredTimerDS > .5f)
            {
                staggeredTimerDS = .0
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

        // spin cube from flick velocity
        val deltaTimeSeconds = (deltaTimeDS * .1)
        if(rotationVelocity != 0f) {
            rotateCube(rotationVelocity * deltaTimeSeconds)
            rotationVelocity *= rotationDragPercent
            if((rotationVelocity < 0 && rotationVelocity > -.001) ||
                (rotationVelocity > 0 && rotationVelocity < rotationStopVelocity)) {
                rotationVelocity = 0f
            }
        }

        // move camera forward based on flick velocity
        if(cameraForwardVelocity != 0.0) {
            moveCameraForward(cameraForwardVelocity * deltaTimeSeconds)
            cameraForwardVelocity *= cameraForwardDragPercent
            if((cameraForwardVelocity < 0.0 && cameraForwardVelocity > -.001) ||
                (cameraForwardVelocity > 0.0 && cameraForwardVelocity < cameraForwardStopVelocity)) {
                cameraForwardVelocity = 0.0
            }
        }

        // rotate cube over time
        val cubeElapsedTimeRotationMat = Mat3.rotate(elapsedTimeDS * cubeRotationAnglePerDS, cubeAutoRotationAxis)
        val cubeModelMat = cubePanRotationMat * cubeElapsedTimeRotationMat * cubeScaleMatrix

        // draw cube outline
        glBindVertexArray(cubeVAO)
        cubeOutlineProgram.use()
        cubeOutlineProgram.setUniform(uniform.viewMat, viewMat)
        cubeOutlineProgram.setUniform(uniform.modelMat, cubeModelMat)
        glDrawElements( GL_TRIANGLES, cubePosTextNormNumVertices, GL_UNSIGNED_INT, 0 /* offset in the EBO*/)

        // draw texture within cube
        textureCropProgram.use()
        textureCropProgram.setUniform(uniform.viewMat, viewMat)
        textureCropProgram.setUniform(uniform.modelMat, cubeModelMat)
        textureCropProgram.setUniform(uniform.diffuseTexture, previousFrameBufferIndex)
        glDrawElements(GL_TRIANGLES, cubePosTextNormNumVertices, GL_UNSIGNED_INT, 0)

        // === Blit off screen render buffer to screen ====
        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffers[currentFrameBufferIndex].index)
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0)
        glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST)
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

        val projectionMat = Mat4.perspective(fovY.degToRad(), aspectRatio, zNear, zFar)

        textureCropProgram.use()
        textureCropProgram.setUniform(uniform.textureWidth, windowWidth.toFloat())
        textureCropProgram.setUniform(uniform.textureHeight, windowHeight.toFloat())
        textureCropProgram.setUniform(uniform.projectionMat, projectionMat)

        cubeOutlineProgram.use()
        cubeOutlineProgram.setUniform(uniform.projectionMat, projectionMat)
    }

    private fun getTimeColor(time: Double = systemTimeInDeciseconds()) : Vec3 {
        val t = time + timeColorOffset
        val lightR = (sin(t.degToRad()) * .5f) + .5f
        val lightG = (cos(t.degToRad() * .5f)) + .5f
        val lightB = (sin((t + 180f).degToRad()) * .5f) + .5f
        return Vec3(lightR.toFloat(), lightG.toFloat(), lightB.toFloat())
    }

    private fun rotateCube(radians: Double) {
        cubePanRotationMat = Mat3.rotate(radians, cubePanRotationAxis) * cubePanRotationMat
    }

    private fun moveCameraForward(units: Double) {
        val maxPanY = cameraForwardMax - camera.position.z
        val minPanY = cameraForwardMin - camera.position.z
        val scaledAndClampedPanY = clamp(units.toFloat(), minPanY, maxPanY)

        camera.moveForward(scaledAndClampedPanY)
    }

    fun jumpTimeColorOffset() {
        timeColorOffset += 100f
        if(timeColorOffset >= 1000f) timeColorOffset = 0f
    }

    override fun onTouchEvent(event: MotionEvent) {
        scaleGestureDetector.onTouchEvent(event)
        if(!pinchInProgress) { gestureDetector.onTouchEvent(event) }
    }
}
