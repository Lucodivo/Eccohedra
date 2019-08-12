package com.inasweaterpoorlyknit.learnopengl_androidport.scenes

import android.content.Context
import javax.microedition.khronos.egl.EGLConfig

import android.opengl.GLES31.*
import android.opengl.GLSurfaceView
import com.inasweaterpoorlyknit.learnopengl_androidport.Program
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import com.inasweaterpoorlyknit.learnopengl_androidport.initializeCubePosTexNormAttBuffers
import java.nio.IntBuffer
import javax.microedition.khronos.opengles.GL10


class InfiniteCubeRenderer(private val context: Context) : GLSurfaceView.Renderer {

    lateinit var cubeProgram: Program
    lateinit var cubeOutlineProgram: Program
    lateinit var frameBufferProgram: Program

    override fun onSurfaceCreated(unused: GL10, config: EGLConfig) {
        cubeProgram = Program(context, R.raw.pos_norm_tex_vertex_shader, R.raw.crop_center_square_tex_fragment_shader)
        cubeOutlineProgram = Program(context, R.raw.pos_norm_tex_vertex_shader, R.raw.discard_alpha_fragment_shader)
        frameBufferProgram = Program(context, R.raw.frame_buffer_vertex_shader, R.raw.basic_texture_fragment_shader)

        // setup vertex attribute objects
        val cubeVAOBuffer = IntBuffer.allocate(1)
        val cubeVBOBuffer = IntBuffer.allocate(1)
        val cubeEBOBuffer = IntBuffer.allocate(1)
        initializeCubePosTexNormAttBuffers(cubeVAOBuffer, cubeVBOBuffer, cubeEBOBuffer)

        // Set the background frame color
        glClearColor(0.5f, 0.0f, 0.0f, 1.0f)
    }

    override fun onDrawFrame(unused: GL10) {
        // Redraw background color
        glClear(GL_COLOR_BUFFER_BIT)
    }

    override fun onSurfaceChanged(unused: GL10, width: Int, height: Int) {
        glViewport(0, 0, width, height)
    }
}
