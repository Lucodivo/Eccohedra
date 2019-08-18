package com.inasweaterpoorlyknit.learnopengl_androidport.scenes

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Context
import android.opengl.GLSurfaceView
import android.widget.Toast
import android.app.ActivityManager
import android.view.MotionEvent
import glm_.vec2.Vec2

@SuppressLint("ViewConstructor")
class InfiniteCubeScene(context: Activity) : GLSurfaceView(context) {

    private val renderer: InfiniteCubeRenderer

    init {
        val activity: Activity = context
        val activityManager = activity.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager?
        val configurationInfo = activityManager!!.deviceConfigurationInfo
        val openGLESVersion = java.lang.Double.parseDouble(configurationInfo.glEsVersion)

        if (openGLESVersion >= 3.0) {
            // We have at least ES 3.0
            setEGLContextClientVersion(3)
        } else {
            val openGLVersionToast: Toast = Toast.makeText(context, "Device only supports OpenGL $openGLESVersion. \n Scene requires at least OpenGL 3.0", Toast.LENGTH_LONG)
            openGLVersionToast.show()
            activity.finish()
        }

        renderer = InfiniteCubeRenderer(context)

        // Set the Renderer for drawing on the GLSurfaceView
        setRenderer(renderer)

        // Render the view only when there is a change in the drawing data
        //renderMode = RENDERMODE_WHEN_DIRTY
    }

    private val TOUCH_SCALE_FACTOR: Float = 180.0f / 320f
    private var previousX: Float = 0f
    private var previousY: Float = 0f

    override fun onTouchEvent(event: MotionEvent): Boolean {

        // MotionEvent reports input details from the touch screen
        // and other input controls. In this case, you are only
        // interested in events where the touch position changed.

        val x: Float = event.x
        val y: Float = event.y

        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                previousX = x
                previousY = y
            }
            MotionEvent.ACTION_MOVE -> {

                var dx: Float = x - previousX
                var dy: Float = y - previousY

                renderer.pan(Vec2(dx, dy) * TOUCH_SCALE_FACTOR)

                previousX = x
                previousY = y
            }
        }

        return true

        //queueEvent {
        //    renderer.touch()
        //}
        //return true
    }
}
