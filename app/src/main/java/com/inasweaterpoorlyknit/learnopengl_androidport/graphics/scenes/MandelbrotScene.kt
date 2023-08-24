package com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes

import android.content.Context
import android.content.SharedPreferences
import android.opengl.GLES20.*
import android.opengl.GLES30.glBindVertexArray
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import androidx.core.math.MathUtils.clamp
import com.inasweaterpoorlyknit.Mat2
import com.inasweaterpoorlyknit.Vec2
import com.inasweaterpoorlyknit.Vec3
import com.inasweaterpoorlyknit.learnopengl_androidport.*
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.*
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10


class MandelbrotScene(context: Context) : Scene(context), ScaleGestureDetector.OnScaleGestureListener, SharedPreferences.OnSharedPreferenceChangeListener {

    // The Mandelbrot scene has two pre-defined colors
    // - Black: The point is in the Mandelbrot set
    // - White: The point's starting point couldn't even be considered for the Mandelbrot set
    // The "colorSub" represents the factors in which colors are taken away from White on their way to Black
    data class Color(val name: String, val colorSub: Vec3)

    companion object {
        private object uniform {
            const val viewPortResolution = "viewPortResolution"
            const val colorSub = "colorSub"
            const val zoom = "zoom"
            const val centerOffset = "centerOffset"
            const val rotationMat = "rotationMat"
        }

        val colors = arrayOf(
            Color("🔴", Vec3(1f, 4f, 2f)), // red
            Color("🟢", Vec3(5f, 1f, 4f)), // green
            Color("🔵", Vec3(6f, 3f, 1f)) // blue
        )
        const val defaultColorIndex = 0

        private const val baseZoom = .25f
        private const val minZoom = .15f
        private const val maxZoom = 130000f // TODO: Expand max if zoom is every expanded beyond current capabilities
    }

    private lateinit var mandelbrotProgram: Program
    private var quadVAO: Int = -1

    private var zoom = baseZoom
    private var centerOffset = Vec2(0f, 0f)
    private var lastFrameRotationMatrix = Mat2(1f)

    // Motion event variables
    private var postPinchZoom_panFlushRequired = false;
    private var prevScaleGestureFocus: Vec2 = Vec2(0f, 0f)
    private var previousX: Float = 0f
    private var previousY: Float = 0f

    private var colorSubIndex: Int
    private var prevColorSubIndex: Int

    // TODO: Consider handling all scenarios through custom RotateGestureDetector?
    private var scaleGestureDetector: ScaleGestureDetector
    private val rotateGestureDetector = RotateGestureDetector()

    init {
        scaleGestureDetector = ScaleGestureDetector(context, this)
        scaleGestureDetector.isQuickScaleEnabled = true
        scaleGestureDetector.isStylusScaleEnabled = true

        val sharedPreferences = context.getSharedPreferences()
        colorSubIndex = sharedPreferences.getMandelbrotColorIndex()
        prevColorSubIndex = colorSubIndex
        sharedPreferences.registerOnSharedPreferenceChangeListener(this)
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        mandelbrotProgram = Program(context, R.raw.uv_coord_vertex_shader, R.raw.mandelbrot_fragment_shader)

        // setup vertex attributes for quad
        val quadVAOBuffer = IntBuffer.allocate(1)
        val quadVBOBuffer = IntBuffer.allocate(1)
        val quadEBOBuffer = IntBuffer.allocate(1)
        initializeFrameBufferQuadVertexAttBuffers(quadVAOBuffer, quadVBOBuffer, quadEBOBuffer)
        quadVAO = quadVAOBuffer[0]

        glClearColor(Vec3(1f, 0f, 0f))

        mandelbrotProgram.use()
        glBindVertexArray(quadVAO)
        mandelbrotProgram.setUniform(uniform.viewPortResolution, windowWidth.toFloat(), windowHeight.toFloat())
        mandelbrotProgram.setUniform(uniform.colorSub, colors[colorSubIndex].colorSub)
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        super.onSurfaceChanged(gl, width, height)

        mandelbrotProgram.use()
        mandelbrotProgram.setUniform(uniform.viewPortResolution, width.toFloat(), height.toFloat())
    }

    override fun onDrawFrame(gl: GL10?) {
        // NOTE: OpenGL calls must be called within specified call back functions
        // Calling OpenGL functions in other functions will surely result in bugs
        val rotationMat = rotationMat2D(rotateGestureDetector.lifetimeRotation)
        lastFrameRotationMatrix = rotationMat

        glClear(GL_COLOR_BUFFER_BIT)

        if(colorSubIndex != prevColorSubIndex) {
            prevColorSubIndex = colorSubIndex
            mandelbrotProgram.setUniform(uniform.colorSub, colors[colorSubIndex].colorSub)
        }
        mandelbrotProgram.setUniform(uniform.zoom, zoom)
        mandelbrotProgram.setUniform(uniform.centerOffset, centerOffset)
        mandelbrotProgram.setUniform(uniform.rotationMat, rotationMat)
        glDrawElements(GL_TRIANGLES, frameBufferQuadNumVertices, GL_UNSIGNED_INT, 0) // offset in the EBO
    }

    private fun pan(deltaX: Float, deltaY: Float) {
        // y is negated because screen coordinates are positive going down
        var centerDelta = Vec2(deltaX / zoom, -deltaY / zoom)
        centerDelta = lastFrameRotationMatrix * centerDelta
        // the value is SUBTRACTED from the center/origin
        // This is because the center represents the center of the mandelbrot set NOT the center of the camera
        // Instead of moving the camera 2 units left, we move the mandelbrot set 2 units right and get the desired result
        centerOffset -= centerDelta
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        rotateGestureDetector.onTouchEvent(event)
        scaleGestureDetector.onTouchEvent(event)

        if(scaleGestureDetector.isInProgress || rotateGestureDetector.isActive) {
            return super.onTouchEvent(event)
        }

        // single pointer gesture events
        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                previousX = event.x
                previousY = event.y

                return true
            }
            MotionEvent.ACTION_MOVE -> {
                if(postPinchZoom_panFlushRequired) {
                    postPinchZoom_panFlushRequired = false
                } else {
                    val dx: Float = event.x - previousX
                    val dy: Float = event.y - previousY
                    pan(dx, dy)
                }

                previousX = event.x
                previousY = event.y
                return true
            }
            MotionEvent.ACTION_UP -> {
                return true
            }
            else -> {
                return super.onTouchEvent(event)
            }
        }
    }

    override fun onSharedPreferenceChanged(sharedPreferences: SharedPreferences, key: String) {
        // NOTE: This could become costly if SharedPreferences are being edited all the time
        if(key == SharedPrefKeys.mandelbrotScene) {
            colorSubIndex = resolveColorIndex(sharedPreferences)
        }
    }

    // Ensuring index is never out of bounds
    private fun resolveColorIndex(sharedPreferences: SharedPreferences): Int {
        val newColorIndex = sharedPreferences.getMandelbrotColorIndex()
        if(newColorIndex < 0 || newColorIndex >= colors.size) {
            sharedPreferences.setMandelbrotColorIndex(defaultColorIndex)
            return defaultColorIndex
        }
        return newColorIndex
    }

    override fun onScale(detector: ScaleGestureDetector): Boolean {
        // zoom
        zoom *= scaleGestureDetector.scaleFactor
        zoom = clamp(zoom, minZoom, maxZoom)

        // pan
        val dx: Float = detector.focusX - prevScaleGestureFocus.x
        val dy: Float = detector.focusY - prevScaleGestureFocus.y
        prevScaleGestureFocus = Vec2(detector.focusX, detector.focusY)
        pan(dx, dy)

        return true
    }

    override fun onScaleBegin(detector: ScaleGestureDetector): Boolean {
        prevScaleGestureFocus = Vec2(detector.focusX, detector.focusY)
        return true
    }

    override fun onScaleEnd(detector: ScaleGestureDetector) {
        postPinchZoom_panFlushRequired = true
    }
}