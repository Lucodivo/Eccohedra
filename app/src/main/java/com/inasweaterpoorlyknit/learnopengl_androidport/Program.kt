package com.inasweaterpoorlyknit.learnopengl_androidport

import android.content.Context
import android.opengl.GLES31.*
import android.support.annotation.RawRes
import android.util.Log
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.createFragmentShader
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.createVertexShader

class Program(context: Context, @RawRes vertexShaderResId: Int, @RawRes fragmentShaderResId: Int)
{
    val id: Int

    init {
        val vertexShader = createVertexShader(context, vertexShaderResId)
        val fragmentShader = createFragmentShader(context, fragmentShaderResId)

        id = glCreateProgram()
        glAttachShader(id, vertexShader)
        glAttachShader(id, fragmentShader)
        glLinkProgram(id)

        val linkSuccess = IntArray(1)
        glGetProgramiv(id, GL_LINK_STATUS, linkSuccess, 0)
        if(linkSuccess[0] == 0) {
            Log.d("Program", glGetProgramInfoLog(id))
            throw RuntimeException("Program failed to setup")
        }

        glDeleteShader(vertexShader)
        glDeleteShader(fragmentShader)
    }

    fun use() = glUseProgram(id)

    // float, mat4, integer
    fun setUniform(name: String, value: Float) = glUniform1f(glGetUniformLocation(id, name), value)
    fun setUniform(name: String, value: Int) = glUniform1i(glGetUniformLocation(id, name), value)
    fun setUniform(name: String, value: FloatArray) = glUniform4fv(glGetUniformLocation(id, name), 1, value, 0)
}