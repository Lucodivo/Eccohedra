package com.inasweaterpoorlyknit.learnopengl_androidport.scenes

import android.content.Context
import android.opengl.GLES20
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
    protected var viewportHeight: Int = -1
    protected var viewportWidth: Int = -1

    protected var orientation: Int = context.resources.configuration.orientation

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        viewportWidth = width
        viewportHeight = height

        GLES20.glViewport(0, 0, viewportWidth, viewportHeight)
    }

    override fun onOrientationChange(orientation: Int) {
        this.orientation = orientation
    }
}