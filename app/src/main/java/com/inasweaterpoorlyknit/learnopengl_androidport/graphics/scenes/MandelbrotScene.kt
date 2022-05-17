package com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes

import android.content.Context
import android.opengl.GLES20.*
import android.opengl.GLES30.glBindVertexArray
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import androidx.core.math.MathUtils.clamp
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.*
import glm_.vec2.Vec2
import glm_.vec3.Vec3
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class MandelbrotScene(context: Context) : Scene(context), ScaleGestureDetector.OnScaleGestureListener {

    private lateinit var mandelbrotProgram: Program
    private var quadVAO: Int = -1

    private val baseZoom = 0.25f
    private val minZoom = 0.15f
    private val maxZoom = 130000.0f // TODO: Expand max if zoom is every expanded beyond current capabilities
    private var zoom = baseZoom
    private var centerOffset = Vec2(0.0f, 0.0f)

    // Motion event variables
    private var postPinchZoom_panFlushRequired = false;
    private var prevScaleGestureFocus: Vec2 = Vec2(0.0f, 0.0f)
    private var previousX: Float = 0.0f
    private var previousY: Float = 0.0f

    private var scaleGestureDetector: ScaleGestureDetector

    init {
        scaleGestureDetector = ScaleGestureDetector(context, this)
        scaleGestureDetector.isQuickScaleEnabled = true
        scaleGestureDetector.isStylusScaleEnabled = true
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        mandelbrotProgram = Program(context, R.raw.uv_coord_vertex_shader, R.raw.mandelbrot_fragment_shader)

        // setup vertex attributes for quad
        val quadVAOBuffer = IntBuffer.allocate(1)
        val quadVBOBuffer = IntBuffer.allocate(1)
        val quadEBOBuffer = IntBuffer.allocate(1)
        initializeFrameBufferQuadVertexAttBuffers(quadVAOBuffer, quadVBOBuffer, quadEBOBuffer)
        quadVAO = quadVAOBuffer[0]

        glClearColor(Vec3(1.0f, 0.0f, 0.0f))

        mandelbrotProgram.use()
        glBindVertexArray(quadVAO)
        mandelbrotProgram.setUniform("viewPortResolution", Vec2(windowWidth, windowHeight))
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        super.onSurfaceChanged(gl, width, height)

        mandelbrotProgram.use()
        mandelbrotProgram.setUniform("viewPortResolution", Vec2(width, height))
    }

    override fun onDrawFrame(gl: GL10?) {
        // NOTE: OpenGL calls must be called within specified call back functions
        // Calling OpenGL functions in other functions will surely result in bugs
        glClear(GL_COLOR_BUFFER_BIT)

        mandelbrotProgram.setUniform("zoom", zoom)
        mandelbrotProgram.setUniform("centerOffset", centerOffset)
        glDrawElements(GL_TRIANGLES, // drawing mode
            6, // number of elements to draw (3 vertices per triangle * 2 triangles per quad)
            GL_UNSIGNED_INT, // type of the indices
            0) // offset in the EBO
    }

    private fun pan(deltaX: Float, deltaY: Float) {
        centerOffset = centerOffset.minus(Vec2(deltaX / zoom, -deltaY / zoom))
    }

    override fun onTouchEvent(motionEvent: MotionEvent): Boolean {
        scaleGestureDetector.onTouchEvent(motionEvent)

        val x: Float = motionEvent.x
        val y: Float = motionEvent.y

        if(scaleGestureDetector.isInProgress) {
            return super.onTouchEvent(motionEvent)
        }

        when (motionEvent.action) {
            MotionEvent.ACTION_DOWN -> {
                previousX = x
                previousY = y

                return true
            }
            MotionEvent.ACTION_MOVE -> {
                if(postPinchZoom_panFlushRequired) {
                    postPinchZoom_panFlushRequired = false
                } else {
                    val dx: Float = x - previousX
                    val dy: Float = y - previousY
                    pan(dx, dy)
                }

                previousX = x
                previousY = y
                return true
            }
            MotionEvent.ACTION_UP -> {
                return true
            }
            else -> {
                return super.onTouchEvent(motionEvent)
            }
        }
    }

    override fun onScale(detector: ScaleGestureDetector): Boolean {
        // zoom
        zoom *= scaleGestureDetector.scaleFactor
        zoom = clamp(zoom, minZoom, maxZoom)

        // pan
        val dx: Float = detector.focusX - prevScaleGestureFocus.x
        val dy: Float = detector.focusY - prevScaleGestureFocus.y
        prevScaleGestureFocus.x = detector.focusX
        prevScaleGestureFocus.y = detector.focusY
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