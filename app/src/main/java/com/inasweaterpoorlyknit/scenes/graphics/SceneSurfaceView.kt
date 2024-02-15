package com.inasweaterpoorlyknit.scenes.graphics

import android.annotation.SuppressLint
import android.content.Context
import android.content.res.Configuration
import android.opengl.GLSurfaceView
import android.view.MotionEvent

@SuppressLint("ViewConstructor")
class SceneSurfaceView(context: Context, private val scene: Scene) : GLSurfaceView(context) {

    init {
        setEGLContextClientVersion(3)
        setRenderer(this.scene)
    }

    @SuppressLint("ClickableViewAccessibility")
    override fun onTouchEvent(event: MotionEvent): Boolean {
        scene.onTouchEvent(event)
        return true
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        when(newConfig.orientation) {
            Configuration.ORIENTATION_PORTRAIT -> scene.onOrientationChange(Orientation.Portrait)
            Configuration.ORIENTATION_LANDSCAPE -> scene.onOrientationChange(Orientation.Landscape)
            else -> {}
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
