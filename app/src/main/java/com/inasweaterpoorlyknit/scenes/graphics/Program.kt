package com.inasweaterpoorlyknit.scenes.graphics

import android.content.res.Resources
import android.opengl.GLES32.GL_LINK_STATUS
import android.opengl.GLES32.glAttachShader
import android.opengl.GLES32.glCreateProgram
import android.opengl.GLES32.glDeleteShader
import android.opengl.GLES32.glGetProgramInfoLog
import android.opengl.GLES32.glGetProgramiv
import android.opengl.GLES32.glGetUniformLocation
import android.opengl.GLES32.glLinkProgram
import android.opengl.GLES32.glUniform1f
import android.opengl.GLES32.glUniform1i
import android.opengl.GLES32.glUniform2f
import android.opengl.GLES32.glUniform2fv
import android.opengl.GLES32.glUniform3f
import android.opengl.GLES32.glUniform3fv
import android.opengl.GLES32.glUniform4f
import android.opengl.GLES32.glUniform4fv
import android.opengl.GLES32.glUniformMatrix2fv
import android.opengl.GLES32.glUniformMatrix3fv
import android.opengl.GLES32.glUniformMatrix4fv
import android.opengl.GLES32.glUseProgram
import android.util.Log
import androidx.annotation.RawRes
import com.inasweaterpoorlyknit.noopmath.Mat2
import com.inasweaterpoorlyknit.noopmath.Mat3
import com.inasweaterpoorlyknit.noopmath.Mat4
import com.inasweaterpoorlyknit.noopmath.Vec2
import com.inasweaterpoorlyknit.noopmath.Vec3
import com.inasweaterpoorlyknit.noopmath.Vec4

class Program(resources: Resources, @RawRes vertexShaderResId: Int, @RawRes fragmentShaderResId: Int)
{
    private val id: Int

    init {
        val vertexShader = createVertexShader(resources, vertexShaderResId)
        val fragmentShader = createFragmentShader(resources, fragmentShaderResId)

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
}