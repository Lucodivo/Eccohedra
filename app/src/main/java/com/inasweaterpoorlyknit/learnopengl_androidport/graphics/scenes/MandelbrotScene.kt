package com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes

import android.content.Context
import android.content.SharedPreferences
import android.opengl.GLES32.*
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import androidx.core.math.MathUtils.clamp
import com.inasweaterpoorlyknit.Mat2
import com.inasweaterpoorlyknit.Vec2
import com.inasweaterpoorlyknit.dVec2
import com.inasweaterpoorlyknit.Vec3
import com.inasweaterpoorlyknit.learnopengl_androidport.*
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.*
import com.inasweaterpoorlyknit.lerp
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import kotlin.math.min
import kotlin.math.pow


class MandelbrotScene(context: Context) : Scene(context), ScaleGestureDetector.OnScaleGestureListener, SharedPreferences.OnSharedPreferenceChangeListener {

    companion object {
        private object uniform {
            const val viewPortResolution = "viewPortResolution"
            const val accentColors = "accentColors"
            const val accentColor1 = "accentColor1"
            const val accentColor2 = "accentColor2"
            const val zoom = "zoom"
            const val maxIterations = "maxIterations"
            const val centerOffset = "centerOffset"
            const val rotationMat = "rotationMat"
        }

        data class AccentColor(val name: String, val accentColor1: Vec3, val accentColor2: Vec3)
        val colors = arrayOf(
            AccentColor("🎭", Vec3(38f/255f, 82f/255f, 237f/255f), Vec3(228f/255f, 133f/255f, 0f/255f)), // blue/yellow
            AccentColor("🏳️‍⚧️", Vec3(245f/255f, 171f/255f, 185f/255f), Vec3(91f/255f, 207f/255f, 250f/255f)), // pastel blue/pink
            AccentColor("💗", Vec3(231f/255f, 90f/255f, 100f/255f), Vec3(168f/255f, 43f/255f, 204f/255f)), // pink/purple
            AccentColor("🏜️", Vec3(0.9019608f, 0.59607846f, 0.0f), Vec3(0.81441176f, 0.11088236f, 0.26382354f)) // orange/yellow
        )
        const val defaultColorIndex = 0

        private const val minZoom: Double = .25 // 1 means that we can see -0.5 to 0.5 in the minimum dimension
        private const val maxZoom: Double = 130000.0 // TODO: Expand max if zoom is every expanded beyond current capabilities
        private const val baseZoom: Double = minZoom
    }

    private lateinit var mandelbrotProgram: Program
    private var quadVAO: Int = -1

    private var zoom = baseZoom
    private var centerOffset = dVec2(0.0, 0.0) //Vec2HighP(-1.70, 0.0)
    private var frameRotationMatrix = Mat2(1f)

    // Motion event variables
    private var postPinchZoom_panFlushRequired = false
    private var prevScaleGestureFocus: Vec2 = Vec2(0f, 0f)
    private var previousX: Float = 0f
    private var previousY: Float = 0f
    private var doubleTapInProgress = false

    private var accentColorsIndex: Int
    private var pixelsPerUnit: Double = 0.0

    // TODO: Consider handling all scenarios through custom RotateGestureDetector?
    private var scaleGestureDetector: ScaleGestureDetector
    private var gestureDetector: GestureDetector
    private val rotateGestureDetector = RotateGestureDetector()

    init {
        scaleGestureDetector = ScaleGestureDetector(context, this)
        scaleGestureDetector.isQuickScaleEnabled = false // Default implementation of quick scaling feels awful
        gestureDetector = GestureDetector(context, object : GestureDetector.OnGestureListener {
            override fun onDown(e: MotionEvent) = true
            override fun onShowPress(e: MotionEvent){}
            override fun onSingleTapUp(e: MotionEvent) = true
            override fun onScroll(e1: MotionEvent?, e2: MotionEvent, distanceX: Float, distanceY: Float) = true
            override fun onLongPress(e: MotionEvent){}
            override fun onFling(e1: MotionEvent?, e2: MotionEvent, velocityX: Float, velocityY: Float) = true
        })
        gestureDetector.setOnDoubleTapListener(object : GestureDetector.OnDoubleTapListener {
            override fun onSingleTapConfirmed(e: MotionEvent): Boolean {
                doubleTapInProgress = false
                return true
            }

            override fun onDoubleTap(e: MotionEvent): Boolean {
                doubleTapInProgress = true
                return true
            }

            override fun onDoubleTapEvent(e: MotionEvent): Boolean {
                doubleTapInProgress = true
                return true
            }
        })
        val sharedPreferences = context.getSharedPreferences()
        accentColorsIndex = sharedPreferences.getMandelbrotColorIndex()
        sharedPreferences.registerOnSharedPreferenceChangeListener(this)
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        mandelbrotProgram = Program(context, R.raw.mandelbrot_vertex_shader, R.raw.mandelbrot_fragment_shader)

        pixelsPerUnit = min(windowWidth, windowHeight).toDouble()

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
        mandelbrotProgram.setUniform(uniform.accentColor1, colors[accentColorsIndex].accentColor1)
        mandelbrotProgram.setUniform(uniform.accentColor2, colors[accentColorsIndex].accentColor2)

        mandelbrotProgram.setUniform(uniform.maxIterations, 1000)
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        super.onSurfaceChanged(gl, width, height)

        pixelsPerUnit = min(windowWidth, windowHeight).toDouble()

        mandelbrotProgram.use()
        mandelbrotProgram.setUniform(uniform.viewPortResolution, width.toFloat(), height.toFloat())

    }

    override fun onDrawFrame(gl: GL10?) {
        // NOTE: OpenGL calls must be called within specified call back functions
        // Calling OpenGL functions in other functions will result in bugs

        // The mandelbrot scene is unique in that it doesn't need to redraw if there are no changes in zoom, accent color index, center offset or rotation
        // This makes it a great candidate for preventing additional draws. However, we do not have control over swapping of buffers.
        // A potential way to optimize this would be to draw to a separate buffer that always gets copied to the display buffer but in which that separate buffer
        // is only updated when the changes noted above occur

        val rotation = rotateGestureDetector.lifetimeRotation

        frameRotationMatrix = rotationMat2D(rotation)

        glClear(GL_COLOR_BUFFER_BIT)

        mandelbrotProgram.setUniform(uniform.zoom, zoom.toFloat())
        mandelbrotProgram.setUniform(uniform.centerOffset, centerOffset.x.toFloat(), centerOffset.y.toFloat())
        mandelbrotProgram.setUniform(uniform.rotationMat, frameRotationMatrix)
        glDrawElements(GL_TRIANGLES, frameBufferQuadNumVertices, GL_UNSIGNED_INT, 0) // offset in the EBO
    }

    private fun pan(deltaX: Double, deltaY: Double) {
        // y is negated because screen coordinates are positive going down
        var centerDelta = dVec2(deltaX / (zoom * pixelsPerUnit), -deltaY / (zoom * pixelsPerUnit))
        centerDelta = dVec2(frameRotationMatrix * centerDelta.toVec2())
        // the value is SUBTRACTED from the center/origin
        // This is because the center represents the center of the mandelbrot set NOT the center of the camera
        // Instead of moving the camera 2 units left, we move the mandelbrot set 2 units right and get the desired result
        centerOffset -= centerDelta

        centerOffset = dVec2(
            clamp(centerOffset.x, -2.0, 2.0),
            clamp(centerOffset.y, -2.0, 2.0)
        )
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        rotateGestureDetector.onTouchEvent(event)
        scaleGestureDetector.onTouchEvent(event)
        gestureDetector.onTouchEvent(event)

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
                val dx: Double = event.x.toDouble() - previousX.toDouble()
                val dy: Double = event.y.toDouble() - previousY.toDouble()
                previousX = event.x
                previousY = event.y

                if(postPinchZoom_panFlushRequired) {
                    // When one finger is pulled off of a pinch to zoom, that pinch to zoom event ends but the single finger event continues.
                    // The initial result causes a MotionEvent with a huge delta position. This aims to ignore this MotionEvent.
                    postPinchZoom_panFlushRequired = false;
                    return true;
                }

                if(doubleTapInProgress) {
                    // normalize delta y to be between 0 to 2
                    val dyNormalized = (dy + windowHeight) / windowHeight
                    val factor = dyNormalized.pow(4)
                    scaleZoom(factor.toFloat())
                } else {
                    pan(dx, dy)
                }
                return true
            }
            MotionEvent.ACTION_UP -> {
                doubleTapInProgress = false
                return true
            }
            else -> {
                return super.onTouchEvent(event)
            }
        }
    }

    override fun onSharedPreferenceChanged(sharedPreferences: SharedPreferences, key: String?) {
        // NOTE: This could become costly if SharedPreferences are being edited all the time
        if(key == SharedPrefKeys.mandelbrotScene) {
            accentColorsIndex = resolveColorIndex(sharedPreferences)
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

    private fun scaleZoom(factor: Float) {
        zoom = clamp(zoom * factor, minZoom, maxZoom)
    }

    override fun onScale(detector: ScaleGestureDetector): Boolean {
        // zoom
        scaleZoom(detector.scaleFactor)

        // pan
        val dx: Double = detector.focusX.toDouble() - prevScaleGestureFocus.x.toDouble()
        val dy: Double = detector.focusY.toDouble() - prevScaleGestureFocus.y.toDouble()
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