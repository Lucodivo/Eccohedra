package com.inasweaterpoorlyknit.learnopengl_androidport.scenes

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Context
import android.opengl.GLSurfaceView
import android.widget.Toast
import android.app.ActivityManager
import android.view.MotionEvent


@SuppressLint("ViewConstructor")
class SceneSurfaceView(context: Activity, private var scene: Scene) : GLSurfaceView(context) {

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

        setRenderer(scene)
    }

    @SuppressLint("ClickableViewAccessibility")
    override fun onTouchEvent(event: MotionEvent): Boolean {
        return scene.onTouchEvent(event) || super.onTouchEvent(event)
    }

    override fun onAttachedToWindow() {
        super.onAttachedToWindow()
        scene.onAttach()
    }

    override fun onDetachedFromWindow() {
        super.onDetachedFromWindow()
        scene.onDetach()
    }

    fun orientationChange(orientation: Int) {
        scene.onOrientationChange(orientation)
    }
}
