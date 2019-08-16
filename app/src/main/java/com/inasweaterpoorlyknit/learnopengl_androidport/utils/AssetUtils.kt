package com.inasweaterpoorlyknit.learnopengl_androidport.utils

import android.content.Context
import android.graphics.BitmapFactory
import android.opengl.GLES20.*
import android.support.annotation.RawRes
import java.io.InputStream
import java.nio.IntBuffer

fun getResourceRawFileAsString(context: Context, @RawRes id: Int): String {
    return context.resources.openRawResource(id).readAsString()
}

// NOTE: This function also closes the InputStream
fun InputStream.readAsString(): String {
    val inputAsString = java.util.Scanner(this).useDelimiter("\\A")
    val result = if (inputAsString.hasNext()) inputAsString.next() else ""
    close()
    return result
}

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

    var internalFormat: Int
    var externalFormat: Int

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