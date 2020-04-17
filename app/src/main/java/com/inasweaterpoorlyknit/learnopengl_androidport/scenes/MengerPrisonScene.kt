package com.inasweaterpoorlyknit.learnopengl_androidport.scenes

import android.content.Context
import android.content.res.Configuration
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.opengl.GLES20.GL_UNSIGNED_INT
import android.opengl.GLES20.glClear
import android.opengl.GLES30.*
import android.util.Log
import android.util.Log.DEBUG
import android.view.MotionEvent
import android.widget.Toast
import com.inasweaterpoorlyknit.learnopengl_androidport.Camera
import com.inasweaterpoorlyknit.learnopengl_androidport.Program
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import com.inasweaterpoorlyknit.learnopengl_androidport.initializeFrameBufferQuadVertexAttBuffers
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.MAT_4x4_SIZE
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.glClearColor
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.systemTimeInDeciseconds
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.systemTimeInSeconds
import glm_.mat4x4.Mat4
import glm_.vec2.Vec2
import glm_.vec3.Vec3
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

private const val actionTimeFrame = 0.25f
private const val maxIterations = 5

class MengerPrisonScene(context: Context) : Scene(context), SensorEventListener {

    data class Resolution (
        val width: Int,
        val height: Int
    )

    private val landscape get() = windowWidth > windowHeight

    private lateinit var landscapeResolutions: Array<Resolution>

    private val TAG = MengerPrisonScene::class.java.canonicalName

    private val camera = Camera()
    private lateinit var mengerPrisonProgram: Program
    private var quadVAO: Int = -1

    private var elapsedTime : Double = 0.0
    private var lastFrameTime: Double = -1.0
    private var firstFrameTime: Double = -1.0

    private var actionDownTime: Double = 0.0

    private val sensorManager: SensorManager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
    private val sensor: Sensor? = sensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR)
    private val rotationSensorMatrix: FloatArray = FloatArray(MAT_4x4_SIZE)

    private var cameraSpeed = 0.5f

    private var currentResolutionIndex = 0
    private var prevFrameResolutionIndex: Int = currentResolutionIndex // informs us of a resolution change within onDrawFrame()
    private val resolution get() = landscapeResolutions[currentResolutionIndex]

    private var frameBuffer = IntArray(1)
    private var rbo = IntArray(1)
    private var frameBufferTexture = IntArray(1)

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {

        initializeRotationMat()

        mengerPrisonProgram = Program(context, R.raw.uv_coord_vertex_shader, R.raw.menger_prison_fragment_shader)

        // setup vertex attributes for quad
        val quadVAOBuffer = IntBuffer.allocate(1)
        val quadVBOBuffer = IntBuffer.allocate(1)
        val quadEBOBuffer = IntBuffer.allocate(1)
        initializeFrameBufferQuadVertexAttBuffers(quadVAOBuffer, quadVBOBuffer, quadEBOBuffer)
        quadVAO = quadVAOBuffer[0]

        firstFrameTime = systemTimeInDeciseconds()
        glClearColor(Vec3(1.0f, 0.0f, 0.0f))
        camera.movementSpeed *= 4.0f

        mengerPrisonProgram.use()
        glBindVertexArray(quadVAO)
        mengerPrisonProgram.setUniform("iterations", maxIterations)
        camera.position = Vec3(0.0f, 0.0f, 0.0f)
    }

    private fun initResolutions(width: Int, height: Int) {
        landscapeResolutions = arrayOf(
            Resolution(width / 32, height / 32),
            Resolution(width / 16, height / 16),
            Resolution(width / 8, height / 8),
            Resolution(width / 4, height / 4),
            Resolution(width / 2, height / 2)
        )
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        super.onSurfaceChanged(gl, width, height)
        initResolutions(width, height)
        initializeFrameBuffer(resolution.width, resolution.height)

        mengerPrisonProgram.use()
        mengerPrisonProgram.setUniform("viewPortResolution", Vec2(resolution.width, resolution.height))
    }

    override fun onDrawFrame(gl: GL10?) {
        // NOTE: OpenGL calls must be called within specified call back functions
        // Calling OpenGL functions in other functions will surely result in bugs

        elapsedTime = systemTimeInDeciseconds() - firstFrameTime
        val deltaTime = elapsedTime - lastFrameTime
        lastFrameTime = elapsedTime

        if(prevFrameResolutionIndex != currentResolutionIndex) {
            initializeFrameBuffer(resolution.width, resolution.height)
            prevFrameResolutionIndex = currentResolutionIndex
            mengerPrisonProgram.setUniform("viewPortResolution", Vec2(resolution.width, resolution.height))
        }

        glViewport(0, 0, resolution.width, resolution.height)
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer[0])

        glClear(GL_COLOR_BUFFER_BIT)

        val rotationMat = camera.getRotationMatrix(deltaTime.toFloat())
        camera.position.plusAssign(camera.front * deltaTime * cameraSpeed)
        mengerPrisonProgram.use()
        mengerPrisonProgram.setUniform("rayOrigin", camera.position)
        mengerPrisonProgram.setUniform("viewRotationMat", rotationMat)
        glDrawElements(GL_TRIANGLES, // drawing mode
            6, // number of elements to draw (3 vertices per triangle * 2 triangles per quad)
            GL_UNSIGNED_INT, // type of the indices
            0) // offset in the EBO

        glViewport(0, 0, windowWidth, windowHeight)
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer[0])
        glBlitFramebuffer(0, 0, resolution.width, resolution.height, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST)
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

    private fun action() {
        if(++currentResolutionIndex >= landscapeResolutions.size) {
            currentResolutionIndex = 0
        }
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        return when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                actionDownTime = systemTimeInSeconds()
                true
            }
            MotionEvent.ACTION_UP -> {
                if((systemTimeInSeconds() - actionDownTime) <= actionTimeFrame) {
                    action()
                }
                true
            }
            else -> {
                super.onTouchEvent(event)
            }
        }
    }

    private fun initializeRotationMat() {
        // set rotation matrix to identity matrix
        rotationSensorMatrix[0] = 1.0f
        rotationSensorMatrix[4] = 1.0f
        rotationSensorMatrix[8] = 1.0f
        rotationSensorMatrix[12] = 1.0f
    }

    private fun initializeFrameBuffer(width: Int, height: Int) {

        glDeleteFramebuffers(1, frameBuffer, 0)
        glDeleteRenderbuffers(1, rbo, 0)
        glDeleteTextures(1, frameBufferTexture, 0)

        // creating frame buffer
        glGenFramebuffers(1, frameBuffer, 0)
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer[0])

        // creating frame buffer texture
        glGenTextures(1, frameBufferTexture, 0)
        glBindTexture(GL_TEXTURE_2D, frameBufferTexture[0])
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, null)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glBindTexture(GL_TEXTURE_2D, 0) // unbind

        // attach texture w/ color to frame buffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, // frame buffer we're targeting (draw, read, or both)
            GL_COLOR_ATTACHMENT0, // type of attachment
            GL_TEXTURE_2D, // type of texture
            frameBufferTexture[0], // texture
            0) // mipmap level

        // creating render buffer to be depth/stencil buffer
        glGenRenderbuffers(1, rbo, 0)
        glBindRenderbuffer(GL_RENDERBUFFER, rbo[0])
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height)
        glBindRenderbuffer(GL_RENDERBUFFER, 0) // unbind
        // attach render buffer w/ depth & stencil to frame buffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, // frame buffer target
            GL_DEPTH_STENCIL_ATTACHMENT, // attachment point of frame buffer
            GL_RENDERBUFFER, // render buffer target
            rbo[0])  // render buffer

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            Log.println(DEBUG, TAG,"ERROR::FRAMEBUFFER:: Framebuffer is not complete!")
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0)
    }

    private fun deviceRotation(mat4: Mat4) {
        camera.processRotationSensor(Mat4(mat4))
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