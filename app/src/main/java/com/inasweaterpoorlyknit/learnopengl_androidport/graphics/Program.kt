package com.inasweaterpoorlyknit.learnopengl_androidport.graphics

import android.content.Context
import android.opengl.GLES20.*
import android.util.Log
import androidx.annotation.RawRes
import glm_.mat3x3.Mat3
import glm_.mat4x4.Mat4
import glm_.vec2.Vec2
import glm_.vec3.Vec3
import java.nio.FloatBuffer

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

    // float, mat4, integer
    fun setUniform(name: String, value: Float) = glUniform1f(glGetUniformLocation(id, name), value)
    fun setUniform(name: String, x: Float, y: Float) = glUniform2f(glGetUniformLocation(id, name), x, y)
    fun setUniform(name: String, x: Float, y: Float, z: Float) = glUniform3f(glGetUniformLocation(id, name), x, y, z)
    fun setUniform(name: String, value: Vec2) = setUniform(name, value.x, value.y)
    fun setUniform(name: String, value: Vec3) = setUniform(name, value.x, value.y, value.z)
    fun setUniform(name: String, value: Int) = glUniform1i(glGetUniformLocation(id, name), value)
    fun setUniform(name: String, value: Mat3) = glUniformMatrix3fv(glGetUniformLocation(id, name), 1, false, value to FloatBuffer.allocate(3*3))
    fun setUniform(name: String, value: Mat4) = glUniformMatrix4fv(glGetUniformLocation(id, name), 1, false, value to FloatBuffer.allocate(4*4))
}