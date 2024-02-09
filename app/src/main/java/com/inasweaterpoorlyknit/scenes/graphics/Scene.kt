package com.inasweaterpoorlyknit.scenes.graphics

import android.content.Context
import android.opengl.GLES32.glViewport
import android.opengl.GLSurfaceView
import android.view.MotionEvent
import javax.microedition.khronos.opengles.GL10

interface ExternalInputListener {
    fun onAttach(){}
    fun onDetach(){}
    fun onTouchEvent(event: MotionEvent): Boolean = false
    fun onOrientationChange(orientation: Int){}
}

abstract class Scene(protected val context: Context) : GLSurfaceView.Renderer, ExternalInputListener {
    protected var windowHeight: Int = -1
    protected var windowWidth: Int = -1
    protected val aspectRatio: Double
        get() = windowWidth.toDouble()/windowHeight.toDouble()

    protected val sceneOrientation: Orientation = context.orientation

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        windowWidth = width
        windowHeight = height

        glViewport(0, 0, windowWidth, windowHeight)
    }
}