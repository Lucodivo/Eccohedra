package com.inasweaterpoorlyknit.learnopengl_androidport.utils

import android.content.Context
import android.opengl.GLES31.*
import android.support.annotation.RawRes
import android.util.Log

// NOTE: OpenGL ES does not support geometry shaders

fun createVertexShader(context: Context, @RawRes shaderResourceId: Int): Int {
    return createVertexShader(getResourceRawFileAsString(context, shaderResourceId))
}

fun createFragmentShader(context: Context, @RawRes shaderResourceId: Int): Int {
    return createFragmentShader(getResourceRawFileAsString(context, shaderResourceId))
}

fun createVertexShader(shaderText: String): Int {
    return createShader(shaderText, GL_VERTEX_SHADER) ?: throw RuntimeException("Error creating vertex shader")
}

fun createFragmentShader(shaderText: String): Int {
    return createShader(shaderText, GL_FRAGMENT_SHADER) ?: throw RuntimeException("Error creating fragment shader")
}

// Returns null if shader was not successfully created
fun createShader(shaderText: String, shaderType: Int): Int? {
    val shader = glCreateShader(shaderType)
    if (shader != 0) {
        glShaderSource(shader, shaderText)

        glCompileShader(shader)

        val compileStatus = IntArray(1)
        glGetShaderiv(shader, GL_COMPILE_STATUS, compileStatus, 0)

        // If the compilation failed, delete the shaderText.
        if (compileStatus[0] == 0) {
            Log.e("ShaderUtil", glGetShaderInfoLog(shader))
            glDeleteShader(shader)
            return null
        }
    }

    return shader
}