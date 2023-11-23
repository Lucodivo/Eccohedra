package com.inasweaterpoorlyknit.learnopengl_androidport.graphics

import android.content.Context
import android.opengl.GLES32.*
import android.util.Log
import androidx.annotation.RawRes

import com.inasweaterpoorlyknit.Mat2
import com.inasweaterpoorlyknit.Mat3
import com.inasweaterpoorlyknit.Mat4
import com.inasweaterpoorlyknit.Vec2
import com.inasweaterpoorlyknit.Vec3
import com.inasweaterpoorlyknit.Vec4

class Program(context: Context, @RawRes vertexShaderResId: Int, @RawRes fragmentShaderResId: Int)
{
    private val id: Int

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
            throw RuntimeException("OpenGL program failed to setup.")
        }

        glDeleteShader(vertexShader)
        glDeleteShader(fragmentShader)
    }

    fun use() = glUseProgram(id)

    fun setUniform(name: String, value: Float) = glUniform1f(glGetUniformLocation(id, name), value)
    fun setUniform(name: String, value: Int) = glUniform1i(glGetUniformLocation(id, name), value)

    fun setUniform(name: String, x: Float, y: Float) = glUniform2f(glGetUniformLocation(id, name), x, y)
    fun setUniform(name: String, x: Float, y: Float, z: Float) = glUniform3f(glGetUniformLocation(id, name), x, y, z)
    fun setUniform(name: String, x: Float, y: Float, z: Float, w: Float) = glUniform4f(glGetUniformLocation(id, name), x, y, z, w)

    fun setUniform(name: String, value: Vec2) = glUniform2fv(glGetUniformLocation(id, name), 1, value.elements, 0)
    fun setUniform(name: String, value: Vec3) = glUniform3fv(glGetUniformLocation(id, name), 1, value.elements, 0)
    fun setUniform(name: String, value: Vec4) = glUniform4fv(glGetUniformLocation(id, name), 1, value.elements, 0)

    fun setUniform(name: String, value: Mat2) = glUniformMatrix2fv(glGetUniformLocation(id, name), 1, false, value.elements, 0)
    fun setUniform(name: String, value: Mat3) = glUniformMatrix3fv(glGetUniformLocation(id, name), 1, false, value.elements, 0)
    fun setUniform(name: String, value: Mat4) = glUniformMatrix4fv(glGetUniformLocation(id, name), 1, false, value.elements, 0)

    fun setUniformVec3Array(name: String, value: FloatArray, count: Int = value.size / 3){ glUniform3fv(glGetUniformLocation(id, name), count, value, 0) }
}