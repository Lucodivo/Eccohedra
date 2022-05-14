package com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes

import android.content.Context
import android.opengl.GLES20.*
import android.opengl.GLES20.GL_UNSIGNED_INT
import android.opengl.GLES20.glClear
import android.opengl.GLES30.glBindVertexArray
import android.view.MotionEvent
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.Program
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.Scene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.initializeFrameBufferQuadVertexAttBuffers
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.glClearColor
import com.inasweaterpoorlyknit.learnopengl_androidport.systemTimeInSeconds
import glm_.vec2.Vec2
import glm_.vec3.Vec3
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

private const val zoomScaleSpeed = 0.25f
private const val actionTimeFrame = 0.15f

class MandelbrotScene(context: Context) : Scene(context) {

    private lateinit var mandelbrotProgram: Program
    private var quadVAO: Int = -1

    private var previousX: Float = 0.0f
    private var previousY: Float = 0.0f
    private var actionDownTime: Double = 0.0

    private var zoom = 0.25f
    private var centerOffset = Vec2(0.0f, 0.0f)

    private var lastSingleTapTime : Double = 0.0
    private var doubleTapPan : Boolean = false

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

    override fun onTouchEvent(ev: MotionEvent): Boolean {
        val x: Float = ev.x
        val y: Float = ev.y

        when (ev.action) {
            MotionEvent.ACTION_DOWN -> {
                val time = systemTimeInSeconds()
                if((time - lastSingleTapTime) <= actionTimeFrame) {
                    doubleTapPan = true
                }

                previousX = x
                previousY = y
                actionDownTime = time

                return true
            }
            MotionEvent.ACTION_MOVE -> {

                if(doubleTapPan) {
                    val scaleFactor = (y - previousY) * zoomScaleSpeed
                    val minZoomDelta = zoom * 0.01f
                    val maxZoomDelta = zoom * 0.1f
                    var zoomDelta = minZoomDelta * scaleFactor
                    if(zoomDelta < minZoomDelta && zoomDelta > -minZoomDelta) zoomDelta = 0.0f
                    else if(zoomDelta > maxZoomDelta) zoomDelta = maxZoomDelta
                    else if(zoomDelta < -maxZoomDelta) zoomDelta = -maxZoomDelta
                    zoom += zoomDelta
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
                val time = systemTimeInSeconds()
                if(doubleTapPan) {
                    doubleTapPan = false
                } else if((time - actionDownTime) <= actionTimeFrame) {
                    lastSingleTapTime = time
                }
                return true
            }
            else -> {
                return super.onTouchEvent(ev)
            }
        }
    }
}