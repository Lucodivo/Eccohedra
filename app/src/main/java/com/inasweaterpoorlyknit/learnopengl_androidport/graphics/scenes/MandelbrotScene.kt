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

    // The Mandelbrot scene has two pre-defined colors
    // - Black: The point is in the Mandelbrot set
    // - White: The point's starting point couldn't even be considered for the Mandelbrot set
    // The "colorSub" represents the factors in which colors are taken away from White on their way to Black
    data class Color(val name: String, val colorSub: Vec3)

    companion object {
        private object uniform {
            const val viewPortResolution = "viewPortResolution"
            const val accentColors = "accentColors"
            const val zoom = "zoom"
            const val maxIterations = "maxIterations"
            const val centerOffset = "centerOffset"
            const val rotationMat = "rotationMat"
        }

        val colors = arrayOf(
            Color("🎭", Vec3(.7f, .0f, .0f)), // blue/yellow
            Color("🏳️‍⚧️", Vec3(.0f, .7f, .0f)), // pastel blue/pink
            Color("💗", Vec3(.0f, .0f, 7f)), // pink/
            Color("🏜️", Vec3(.0f))
        )
        const val defaultColorIndex = 0

        val accentColors = arrayOf(
            floatArrayOf( // gold to blue, from wikipedia
                0.035f, 0.004f, 0.184f,
                0.016f, 0.0157f, 0.286f,
                0.0f, 0.027f, 0.392f,
                0.047f, 0.173f, 0.541f,
                0.094f, 0.322f, 0.694f,
                0.224f, 0.49f, 0.82f,
                0.525f, 0.71f, 0.898f,
                0.827f, 0.925f, 0.972f,
                0.945f, 0.914f, 0.749f,
                0.973f, 0.788f, 0.373f,
                1.0f, 0.667f, 0.0f,
                0.8f, 0.502f, 0.0f,
                0.6f, 0.341f, 0.0f,
                0.416f, 0.204f, 0.012f,
                0.259f, 0.118f, 0.059f, // lightest yellow
                0.098f, 0.027f, 0.102f, // lightest blue
            ),
            floatArrayOf( // pastel linear interpolation 2
                0.975f, 0.66104996f, 0.87945f,
                1.0f, 0.678f, 0.902f, // pastel pink (accent color)
                1.0f, 0.73113f, 0.91817f,
                1.0f, 0.78426003f, 0.93434f,
                1.0f, 0.839f, 0.951f,
                0.839f, 0.9235f, 0.951f,
                0.78586996f, 0.898255f, 0.93482995f,
                0.7327399f, 0.87301004f, 0.91866004f,
                0.678f, 0.847f, 0.902f, // pastel blue (accent color)
                0.66104996f, 0.825825f, 0.87945f,
                0.64409995f, 0.80464995f, 0.8569f,
                0.62715f, 0.7834749f, 0.83435f,
                0.6102f, 0.76229995f, 0.8118f,
                0.9f, 0.6102f, 0.8118f,
                0.92499995f, 0.62715f, 0.83435f,
                0.95f, 0.64409995f, 0.8569f,
            ),
            floatArrayOf(
                0.68441176f, 0.2676471f, 0.49705884f,
                0.7019608f, 0.27450982f, 0.50980395f,
                0.75113726f, 0.3942157f, 0.5906863f,
                0.8003137f, 0.5139216f, 0.6715687f,
                0.8509804f, 0.6372549f, 0.754902f,
                0.82941175f, 0.6039216f, 0.85294116f,
                0.77311766f, 0.4732157f, 0.80441177f,
                0.7168236f, 0.3425098f, 0.7558824f,
                0.65882355f, 0.20784314f, 0.7058824f,
                0.64235294f, 0.20264706f, 0.68823534f,
                0.6258824f, 0.19745098f, 0.67058825f,
                0.6094118f, 0.1922549f, 0.65294117f,
                0.59294116f, 0.18705882f, 0.63529414f,
                0.6317647f, 0.24705882f, 0.45882353f,
                0.64931375f, 0.25392157f, 0.47156864f,
                0.6668627f, 0.26078433f, 0.48431373f,
            ),
            floatArrayOf(
                0.87941176f, 0.5811765f, 0.0f,
                0.9019608f, 0.59607846f, 0.0f,
                0.9181372f, 0.6627255f, 0.165f,
                0.9343138f, 0.72937256f, 0.33f,
                0.9509804f, 0.7980392f, 0.5f,
                0.91764706f, 0.5568628f, 0.63529414f,
                0.8904706f, 0.41062745f, 0.51494116f,
                0.8632941f, 0.26439217f, 0.39458823f,
                0.8352941f, 0.11372549f, 0.27058825f,
                0.81441176f, 0.11088236f, 0.26382354f,
                0.7935294f, 0.108039215f, 0.25705883f,
                0.772647f, 0.10519607f, 0.25029412f,
                0.7517647f, 0.10235294f, 0.24352942f,
                0.8117647f, 0.5364706f, 0.0f,
                0.83431375f, 0.5513725f, 0.0f,
                0.8568628f, 0.5662745f, 0.0f,
            )
        )

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
    private var postPinchZoom_panFlushRequired = false;
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
            override fun onScroll(e1: MotionEvent, e2: MotionEvent, distanceX: Float, distanceY: Float) = true
            override fun onLongPress(e: MotionEvent){}
            override fun onFling(e1: MotionEvent, e2: MotionEvent, velocityX: Float, velocityY: Float) = true
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
        mandelbrotProgram.setUniformVec3Array(uniform.accentColors, accentColors[accentColorsIndex])
        mandelbrotProgram.setUniform(uniform.maxIterations, 100)
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

        mandelbrotProgram.setUniformVec3Array(uniform.accentColors, accentColors[accentColorsIndex])
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
                if(postPinchZoom_panFlushRequired) {
                    postPinchZoom_panFlushRequired = false
                } else {
                    val dx: Double = event.x.toDouble() - previousX.toDouble()
                    val dy: Double = event.y.toDouble() - previousY.toDouble()
                    if(doubleTapInProgress) {
                        // normalize delta y to be between 0 to 2
                        val dyNormalized = (dy + windowHeight) / windowHeight
                        val factor = dyNormalized.pow(4)
                        scaleZoom(factor.toFloat())
                    } else {
                        pan(dx, dy)
                    }
                }

                previousX = event.x
                previousY = event.y
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

    override fun onSharedPreferenceChanged(sharedPreferences: SharedPreferences, key: String) {
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

// Currently used by developer to generator colors to then hardcode, but can potential be used in future to let user choose any two colors they want.
fun generatorAccent16ColorPalette(accentColor1: Vec3, accentColor2: Vec3): FloatArray {
    // 16 colors
    // Darkest accent color #1
    // 3 Darkest to before accent color #1
    // Accent color #1
    // 2 accent color to before lightest accent color
    // Lightest accent color #1
    // Lightest accent color #2
    // 2 Lightest to before accent color #2
    // accent color #2
    // 3 accent color #2 to before darkest accent color #2
    // Darkest accent color #2

    val white = Vec3(1f, 1f, 1f)
    val black = Vec3(0f, 0f, 0f)

    val darkestAccentCol1 = lerp(accentColor1, black, .1f)
    val darkToAccentCol1_1 = lerp(darkestAccentCol1, accentColor1, .25f)
    val darkToAccentCol1_2 = lerp(darkestAccentCol1, accentColor1, .5f)
    val darkToAccentCol1_3 = lerp(darkestAccentCol1, accentColor1, .75f)

    val lightestAccentCol1 = lerp(accentColor1, white, .5f)
    val accentCol1ToLight_1 = lerp(accentColor1, lightestAccentCol1, .33f)
    val accentCol1ToLight_2 = lerp(accentColor1, lightestAccentCol1, .66f)

    val lightestAccentCol2 = lerp(accentColor2, white, .5f)
    val lightestToAccentCol2_1 = lerp(lightestAccentCol2, accentColor2, .33f)
    val lightestToAccentCol2_2 = lerp(lightestAccentCol2, accentColor2, .66f)

    val darkestAccentCol2 = lerp(accentColor2, black, .1f)
    val accentCol2ToDark_1 = lerp(accentColor2, darkestAccentCol2, .25f)
    val accentCol2ToDark_2 = lerp(accentColor2, darkestAccentCol2, .5f)
    val accentCol2ToDark_3 = lerp(accentColor2, darkestAccentCol2, .75f)

    return floatArrayOf(
        darkToAccentCol1_3.x, darkToAccentCol1_3.y, darkToAccentCol1_3.z,
        accentColor1.x, accentColor1.y, accentColor1.z,
        accentCol1ToLight_1.x, accentCol1ToLight_1.y, accentCol1ToLight_1.z,
        accentCol1ToLight_2.x, accentCol1ToLight_2.y, accentCol1ToLight_2.z,
        lightestAccentCol1.x, lightestAccentCol1.y, lightestAccentCol1.z,
        lightestAccentCol2.x, lightestAccentCol2.y, lightestAccentCol2.z,
        lightestToAccentCol2_1.x, lightestToAccentCol2_1.y, lightestToAccentCol2_1.z,
        lightestToAccentCol2_2.x, lightestToAccentCol2_2.y, lightestToAccentCol2_2.z,
        accentColor2.x, accentColor2.y, accentColor2.z,
        accentCol2ToDark_1.x, accentCol2ToDark_1.y, accentCol2ToDark_1.z,
        accentCol2ToDark_2.x, accentCol2ToDark_2.y, accentCol2ToDark_2.z,
        accentCol2ToDark_3.x, accentCol2ToDark_3.y, accentCol2ToDark_3.z,
        darkestAccentCol2.x, darkestAccentCol2.y, darkestAccentCol2.z,
        darkestAccentCol1.x, darkestAccentCol1.y, darkestAccentCol1.z,
        darkToAccentCol1_1.x, darkToAccentCol1_1.y, darkToAccentCol1_1.z,
        darkToAccentCol1_2.x, darkToAccentCol1_2.y, darkToAccentCol1_2.z,
    )
}