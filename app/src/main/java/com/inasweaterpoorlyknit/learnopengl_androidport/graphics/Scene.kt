package com.inasweaterpoorlyknit.learnopengl_androidport.graphics

import android.content.Context
import android.opengl.GLES20
import android.opengl.GLSurfaceView
import android.view.MotionEvent
import com.inasweaterpoorlyknit.learnopengl_androidport.orientation
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

    protected var orientation: Int = context.orientation

    override fun onSurfaceChanged(gl: GL10, width: Int, height: Int) {
        windowWidth = width
        windowHeight = height

        GLES20.glViewport(0, 0, windowWidth, windowHeight)
    }

    override fun onOrientationChange(orientation: Int) {
        this.orientation = orientation
    }
}