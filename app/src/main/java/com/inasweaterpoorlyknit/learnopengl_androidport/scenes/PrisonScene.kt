package com.inasweaterpoorlyknit.learnopengl_androidport.scenes

import android.content.Context
import android.content.res.Configuration
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.opengl.GLES20.*
import android.opengl.GLES20.GL_UNSIGNED_INT
import android.opengl.GLES20.glClear
import android.opengl.GLES30.glBindVertexArray
import android.widget.Toast
import com.inasweaterpoorlyknit.learnopengl_androidport.Camera
import com.inasweaterpoorlyknit.learnopengl_androidport.Program
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import com.inasweaterpoorlyknit.learnopengl_androidport.initializeFrameBufferQuadVertexAttBuffers
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.MAT_4x4_SIZE
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.glClearColor
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.systemTimeInDeciseconds
import glm_.mat4x4.Mat4
import glm_.vec2.Vec2
import glm_.vec3.Vec3
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10


class PrisonScene(context: Context) : Scene(context), SensorEventListener {

    private val camera = Camera()
    private lateinit var prisonProgram: Program
    private var quadVAO: Int = -1

    private var elapsedTime : Double = 0.0
    private var lastFrameTime: Double = -1.0
    private var firstFrameTime: Double = -1.0

    private val sensorManager: SensorManager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
    private val sensor: Sensor? = sensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR)
    private val rotationSensorMatrix: FloatArray = FloatArray(MAT_4x4_SIZE)

    private var cameraSpeed = 0.5f

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        initializeRotationMat()

        prisonProgram = Program(context, R.raw.uv_coord_vertex_shader, R.raw.prison_fragment_shader)

        // setup vertex attributes for quad
        val quadVAOBuffer = IntBuffer.allocate(1)
        val quadVBOBuffer = IntBuffer.allocate(1)
        val quadEBOBuffer = IntBuffer.allocate(1)
        initializeFrameBufferQuadVertexAttBuffers(quadVAOBuffer, quadVBOBuffer, quadEBOBuffer)
        quadVAO = quadVAOBuffer[0]

        firstFrameTime = systemTimeInDeciseconds()
        glClearColor(Vec3(1.0f, 0.0f, 0.0f))
        camera.movementSpeed *= 4.0f

        prisonProgram.use()
        glBindVertexArray(quadVAO)
        prisonProgram.setUniform("viewPortResolution", Vec2(viewportWidth, viewportHeight))
        camera.position = Vec3(0.0f, 0.0f, 0.0f)
    }

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        super.onSurfaceChanged(gl, width, height)

        prisonProgram.use()
        prisonProgram.setUniform("viewPortResolution", Vec2(width, height))
    }

    override fun onDrawFrame(gl: GL10?) {
        elapsedTime = systemTimeInDeciseconds() - firstFrameTime
        val deltaTime = elapsedTime - lastFrameTime
        lastFrameTime = elapsedTime

        glClear(GL_COLOR_BUFFER_BIT)

        val rotationMat = camera.getRotationMatrix(deltaTime.toFloat())
        camera.position.plusAssign(camera.front * deltaTime * cameraSpeed)
        prisonProgram.setUniform("rayOrigin", camera.position)
        prisonProgram.setUniform("viewRotationMat", rotationMat)
        glDrawElements(GL_TRIANGLES, // drawing mode
            6, // number of elements to draw (3 vertices per triangle * 2 triangles per quad)
            GL_UNSIGNED_INT, // type of the indices
            0) // offset in the EBO
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

    private fun initializeRotationMat() {
        // set rotation matrix to identity matrix
        rotationSensorMatrix[0] = 1.0f
        rotationSensorMatrix[4] = 1.0f
        rotationSensorMatrix[8] = 1.0f
        rotationSensorMatrix[12] = 1.0f
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