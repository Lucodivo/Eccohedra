package com.inasweaterpoorlyknit.scenes.graphics

import android.content.res.Resources
import android.graphics.BitmapFactory
import android.opengl.GLES32.GL_COMPILE_STATUS
import android.opengl.GLES32.GL_FRAGMENT_SHADER
import android.opengl.GLES32.GL_NEAREST
import android.opengl.GLES32.GL_RGB
import android.opengl.GLES32.GL_RGBA
import android.opengl.GLES32.GL_TEXTURE_2D
import android.opengl.GLES32.GL_TEXTURE_MAG_FILTER
import android.opengl.GLES32.GL_UNSIGNED_BYTE
import android.opengl.GLES32.GL_VERTEX_SHADER
import android.opengl.GLES32.glBindTexture
import android.opengl.GLES32.glClearColor
import android.opengl.GLES32.glCompileShader
import android.opengl.GLES32.glCreateShader
import android.opengl.GLES32.glDeleteShader
import android.opengl.GLES32.glGenTextures
import android.opengl.GLES32.glGenerateMipmap
import android.opengl.GLES32.glGetShaderInfoLog
import android.opengl.GLES32.glGetShaderiv
import android.opengl.GLES32.glShaderSource
import android.opengl.GLES32.glTexImage2D
import android.opengl.GLES32.glTexParameteri
import androidx.annotation.RawRes
import com.inasweaterpoorlyknit.Vec3
import java.nio.IntBuffer

// NOTE: OpenGL ES does not support geometry shaders

fun loadTexture(resources: Resources, @RawRes resourceId: Int): Int {
    val textureIntArray = IntArray(1)
    glGenTextures(1, textureIntArray, 0)
    val textureId = textureIntArray[0]

    if (textureId == 0) {
        throw RuntimeException("Error generating OpenGL texture.")
    }

    val options = BitmapFactory.Options()
    options.inScaled = false   // No pre-scaling

    // Read in the resource
    val bitmap = BitmapFactory.decodeResource(resources, resourceId, options) ?: throw RuntimeException("Error decoding image: ${resources.getResourceName(resourceId)}")

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

fun createVertexShader(resources: Resources, @RawRes shaderResourceId: Int): Int =
    createShader(resources, shaderResourceId, GL_VERTEX_SHADER)

fun createFragmentShader(resources: Resources, @RawRes shaderResourceId: Int): Int =
    createShader(resources, shaderResourceId, GL_FRAGMENT_SHADER)

// Throws exception of shader cannot be compiled
fun createShader(resources: Resources, @RawRes shaderResourceId: Int, shaderType: Int): Int {
    val shaderText = getResourceRawFileAsString(resources, shaderResourceId)
    val shader = glCreateShader(shaderType)
    if (shader != 0) {
        glShaderSource(shader, shaderText)

        glCompileShader(shader)

        val compileStatus = IntArray(1)
        glGetShaderiv(shader, GL_COMPILE_STATUS, compileStatus, 0)

        // If the compilation failed, delete the shaderText.
        if (compileStatus[0] == 0) {
            val infoLog = glGetShaderInfoLog(shader)
            glDeleteShader(shader)
            throw RuntimeException("ERROR COMPILING SHADER: ${resources.getResourceName(shaderResourceId)}\n$infoLog")
        }
    }

    return shader
}

fun glClearColor(vec3: Vec3) {
    glClearColor(vec3.r, vec3.g, vec3.b, 1.0f)
}