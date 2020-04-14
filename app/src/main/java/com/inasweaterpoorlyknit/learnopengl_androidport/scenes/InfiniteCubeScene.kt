package com.inasweaterpoorlyknit.learnopengl_androidport.scenes

import android.content.Context
import android.content.res.Configuration
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.opengl.GLES20.*
import android.opengl.GLES30.glBindVertexArray
import android.view.MotionEvent
import android.widget.Toast
import com.inasweaterpoorlyknit.learnopengl_androidport.*
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.MAT_4x4_SIZE
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.loadTexture
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.systemTimeInSeconds
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.systemTimeInDeciseconds
import glm_.glm
import glm_.mat4x4.Mat4
import glm_.vec3.Vec3
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import kotlin.math.cos
import kotlin.math.sin
import glm_.vec2.Vec2

import com.inasweaterpoorlyknit.learnopengl_androidport.utils.glClearColor


const val cubeRotationAngle = 2.5f
val cubeRotationAxis = Vec3(1.0f, 0.3f, 0.5f)
const val outlineTextureIndex = 2
const val SMOOTH_TRANSITIONS = true

class InfiniteCubeScene(context: Context) : Scene(context), SensorEventListener {

    private val camera = Camera()
    private lateinit var cubeProgram: Program
    private lateinit var cubeOutlineProgram: Program
    private lateinit var frameBufferProgram: Program
    private lateinit var frameBuffers: Array<FrameBuffer>
    private var cubeVAO: Int = -1
    private var quadVAO: Int = -1
    private var outlineTextureId: Int = -1
    private var projectionMat = Mat4()

    private var currentFrameBufferIndex: Int = 0
    private var previousFrameBufferIndex: Int = 1
    private var cubeScale: Float = 1.0f

    private var lastFrameTime: Double = -1.0
    private var elapsedTime: Double = 0.0
    private var staggeredTimer: Double = 0.0

    private var timeColorOffset = 0.0f

    private val sensorManager: SensorManager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
    private val sensor: Sensor? = sensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR)
    private val rotationSensorMatrix: FloatArray = FloatArray(MAT_4x4_SIZE)

    private val touchScaleFactor: Float = 180.0f / 320f
    private var previousX: Float = 0.0f
    private var previousY: Float = 0.0f
    private var actionDownTime: Double = 0.0

    override fun onSurfaceCreated(unused: GL10, config: EGLConfig) {
        initializeRotationMat()

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

        lastFrameTime = systemTimeInDeciseconds()
    }

    override fun onDrawFrame(unused: GL10) {
        val t = systemTimeInDeciseconds()
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
            if (staggeredTimer > 0.75f)
            {
                staggeredTimer = 0.0
                previousFrameBufferIndex = currentFrameBufferIndex
                currentFrameBufferIndex = if (currentFrameBufferIndex == 0) 1 else 0
            }
        }

        // set background color
        val timeColor = getTimeColor(elapsedTime)
        glClearColor(timeColor)

        // bind default frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[currentFrameBufferIndex].index[0])

        glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)

        val viewMat = camera.getViewMatrix(deltaDeciseconds.toFloat())

        // draw cube
        // rotate with time
        var cubeModelMatrix = Mat4()
        cubeModelMatrix = glm.scale(
            cubeModelMatrix,
            Vec3(cubeScale)
        )
        cubeModelMatrix = glm.rotate(
            cubeModelMatrix,
            (elapsedTime.toFloat() / 8.0f) * glm.radians(cubeRotationAngle),
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
        super.onSurfaceChanged(gl, width, height)
        frameBuffers = arrayOf(FrameBuffer(), FrameBuffer())

        initializeFrameBuffer(frameBuffers[0], windowWidth, windowHeight)
        initializeFrameBuffer(frameBuffers[1], windowWidth, windowHeight)

        // start frame buffer with single color
        glClearColor(0.5f, 1.0f, 0.5f, 1.0f)

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[0].index[0])
        glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[1].index[0])
        glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT)

        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, frameBuffers[0].textureBufferIndex[0])
        glActiveTexture(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D, frameBuffers[1].textureBufferIndex[0])
        glActiveTexture(GL_TEXTURE0)

        projectionMat = glm.perspective(glm.radians(ZOOM), windowWidth.toFloat()/windowHeight, 0.1f, 100.0f)

        cubeProgram.use()
        cubeProgram.setUniform("texWidth", windowWidth.toFloat())
        cubeProgram.setUniform("texHeight", windowHeight.toFloat())
        cubeProgram.setUniform("projection", projectionMat)

        cubeOutlineProgram.use()
        cubeOutlineProgram.setUniform("projection", projectionMat)
    }

    private fun initializeRotationMat() {
        // set rotation matrix to identity matrix
        rotationSensorMatrix[0] = 1.0f
        rotationSensorMatrix[4] = 1.0f
        rotationSensorMatrix[8] = 1.0f
        rotationSensorMatrix[12] = 1.0f
    }

    private fun getTimeColor(time: Double = systemTimeInDeciseconds()) : Vec3 {
        val t = time + timeColorOffset
        val lightR = (sin(glm.radians(t)) / 2.0f) + 0.5f
        val lightG = (cos(glm.radians(t) / 2.0f)) + 0.5f
        val lightB = (sin(glm.radians(t + 180.0f)) / 2.0f) + 0.5f
        return Vec3(lightR, lightG, lightB)
    }

    private fun pan(vec2: Vec2) {
        camera.processPanWalk(vec2)
    }

    fun action() {
        timeColorOffset += 100.0f
        if(timeColorOffset >= 1000.0f) timeColorOffset = 0.0f
    }

    private fun deviceRotation(mat4: Mat4) {
        camera.processRotationSensor(Mat4(mat4))
    }

    override fun onAttach() {
        // enable our sensor when attached
        if(sensor == null) {
            Toast.makeText(context, R.string.no_rotation_sensor, Toast.LENGTH_LONG).show()
        } else {
            sensorManager.registerListener(this, sensor, SensorManager.SENSOR_DELAY_GAME)
        }
    }

    override fun onDetach() {
        // Turn our sensor off on detached
        sensorManager.unregisterListener(this)
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {

        val x: Float = event.x
        val y: Float = event.y

        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                previousX = x
                previousY = y
                actionDownTime = systemTimeInSeconds()
                return true
            }
            MotionEvent.ACTION_MOVE -> {

                val dx: Float = x - previousX
                val dy: Float = y - previousY

                pan(Vec2(dx, dy) * touchScaleFactor)

                previousX = x
                previousY = y
                return true
            }
            MotionEvent.ACTION_UP -> {
                if((systemTimeInSeconds() - actionDownTime) <= 0.3f) {
                    action()
                }
                return true
            }
            else -> {
                return super.onTouchEvent(event)
            }
        }
    }

    override fun onSensorChanged(event: SensorEvent) {
        when(event.sensor.type){
            Sensor.TYPE_ROTATION_VECTOR -> {
                SensorManager.getRotationMatrixFromVector(rotationSensorMatrix, event.values)
                if(orientation == Configuration.ORIENTATION_LANDSCAPE) {
                    SensorManager.remapCoordinateSystem(rotationSensorMatrix, SensorManager.AXIS_Y, SensorManager.AXIS_MINUS_X, rotationSensorMatrix)
                }
                deviceRotation(Mat4(rotationSensorMatrix))
            }
        }
    }

    override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) {}
}
