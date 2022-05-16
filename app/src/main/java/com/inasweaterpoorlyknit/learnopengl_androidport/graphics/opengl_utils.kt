package com.inasweaterpoorlyknit.learnopengl_androidport.graphics

import android.content.Context
import android.graphics.BitmapFactory
import android.opengl.GLES20.*
import android.util.Log
import androidx.annotation.RawRes
import glm_.vec3.Vec3
import java.nio.IntBuffer

// NOTE: OpenGL ES does not support geometry shaders

fun loadTexture(context: Context, @RawRes resourceId: Int): Int {
    val textureIntArray = IntArray(1)
    glGenTextures(1, textureIntArray, 0)
    val textureId = textureIntArray[0]

    if (textureId == 0) {
        throw RuntimeException("Error generating OpenGL texture.")
    }

    val options = BitmapFactory.Options()
    options.inScaled = false   // No pre-scaling

    // Read in the resource
    val bitmap = BitmapFactory.decodeResource(context.resources, resourceId, options) ?: throw RuntimeException("Error decoding image: ${context.resources.getResourceName(resourceId)}")

    val internalFormat: Int
    val externalFormat: Int
    if(bitmap.hasAlpha()) {
        internalFormat = GL_RGBA
        externalFormat = GL_RGBA
    } else {
        internalFormat = GL_RGB
        externalFormat = GL_RGB
    }

    // Bind to the texture in OpenGL
    glBindTexture(GL_TEXTURE_2D, textureId)

    // Load the bitmap into the bound texture.
    val pixels = IntArray(bitmap.width * bitmap.height)
    bitmap.getPixels(pixels, 0, bitmap.width, 0, 0, bitmap.width, bitmap.height);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, bitmap.width, bitmap.height,
        0, externalFormat, GL_UNSIGNED_BYTE, IntBuffer.wrap(pixels));
    glGenerateMipmap(GL_TEXTURE_2D)

    // Set filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)

    // Recycle the bitmap, since its data has been loaded into OpenGL.
    bitmap.recycle()

    return textureId
}

fun createVertexShader(context: Context, @RawRes shaderResourceId: Int): Int {
    return try {
        createVertexShader(getResourceRawFileAsString(context, shaderResourceId))
    } catch (e: RuntimeException) {
        throw RuntimeException("Error creating vertex shader: \n${context.resources.getResourceName(shaderResourceId)}\n${e.message}")
    }
}

fun createFragmentShader(context: Context, @RawRes shaderResourceId: Int): Int {
    return try {
        createFragmentShader(getResourceRawFileAsString(context, shaderResourceId))
    } catch (e: RuntimeException) {
        throw RuntimeException("Error creating fragment shader: ${context.resources.getResourceName(shaderResourceId)}\n${e.message}")
    }
}

fun createVertexShader(shaderText: String): Int {
    return createShader(shaderText, GL_VERTEX_SHADER) ?: throw RuntimeException("Error creating vertex shader with text: \n$shaderText")
}

fun createFragmentShader(shaderText: String): Int {
    return createShader(shaderText, GL_FRAGMENT_SHADER) ?: throw RuntimeException("Error creating fragment shader with text: \n$shaderText")
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

fun glClearColor(vec3: Vec3) {
    glClearColor(vec3.r, vec3.g, vec3.b, 1.0f)
}