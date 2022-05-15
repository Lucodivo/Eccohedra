package com.inasweaterpoorlyknit.learnopengl_androidport.graphics

import android.annotation.SuppressLint
import android.content.Context
import android.content.res.Configuration
import android.opengl.GLSurfaceView
import android.view.MotionEvent

@SuppressLint("ViewConstructor")
class SceneSurfaceView(context: Context, private val scene: Scene) : GLSurfaceView(context) {

    init {
        // We have at least ES 3.0
        setEGLContextClientVersion(3)
        setRenderer(this.scene)
    }

    @SuppressLint("ClickableViewAccessibility")
    override fun onTouchEvent(event: MotionEvent): Boolean {
        return scene.onTouchEvent(event) || super.onTouchEvent(event)
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        newConfig.orientation.let {
            scene.onOrientationChange(it)
        }
    }

    override fun onAttachedToWindow() {
        super.onAttachedToWindow()
        scene.onAttach()
    }

    override fun onDetachedFromWindow() {
        super.onDetachedFromWindow()
        scene.onDetach()
    }
}
