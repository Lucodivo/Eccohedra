package com.inasweaterpoorlyknit.learnopengl_androidport

import android.opengl.GLES30.*
import android.util.Log
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.BYTES_PER_FLOAT
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.BYTES_PER_INT
import java.nio.FloatBuffer
import java.nio.IntBuffer

// ===== cube values =====
const val cubePosTexNormAttSizeInBytes = 8 * BYTES_PER_FLOAT // 8 times size in bytes
const val cubePosTextNormNumElements = 12 // 2 triangles per side * 6 sides per cube
val cubePosTexNormAttributes = floatArrayOf(
    // positions           // normals            // texture positions
    // face #1
    -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,     // bottom left
    0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 0.0f,    // bottom right
    0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 1.0f,       // top right
    -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,        // top left
    // face #2
    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,    0.0f, 0.0f,     // bottom left
    0.5f, -0.5f,  0.5f,    0.0f,  0.0f, 1.0f,    1.0f, 0.0f,    // bottom right
    0.5f,  0.5f,  0.5f,    0.0f,  0.0f, 1.0f,    1.0f, 1.0f,       // top right
    -0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,    0.0f, 1.0f,        // top left
    // face #3
    -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,    // bottom right
    -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,       // top right
    -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,        // top left
    -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,     // bottom left
    // face #4
    0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f,    // bottom right
    0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 1.0f,       // top right
    0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 1.0f,        // top left
    0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 0.0f,     // bottom left
    // face #5
    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,        // top left
    0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 1.0f,       // top right
    0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 0.0f,    // bottom right
    -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,     // bottom left
    // face #6
    -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,        // top left
    0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,   1.0f, 1.0f,       // top right
    0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,   1.0f, 0.0f,    // bottom right
    -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f      // bottom left
)
val cubePosNormIndices = intArrayOf(
    0, 2, 1,
    2, 0, 3,
    4, 5, 6,
    6, 7, 4,
    8, 9, 10,
    10, 11, 8,
    12, 14, 13,
    14, 12, 15,
    16, 17, 18,
    18, 19, 16,
    20, 22, 21,
    22, 20, 23
)

// ===== frame buffer quad values =====
const val frameBufferQuadVertexAttSizeInBytes = 4 * BYTES_PER_FLOAT
val frameBufferQuadVertexAttributes = floatArrayOf(
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f
)
val quadIndices = intArrayOf(
    0, 1, 2,
    0, 2, 3
)

fun FloatArray.toFloatBuffer(): FloatBuffer {
    val buffer = FloatBuffer.allocate(this.size)
    buffer.put(this)
    buffer.position(0)
    return buffer
}

fun IntArray.toIntBuffer(): IntBuffer {
    val buffer = IntBuffer.allocate(this.size)
    buffer.put(this)
    buffer.position(0)
    return buffer
}

fun initializeCubePosTexNormAttBuffers(vaoIntBuffer: IntBuffer, vboIntBuffer: IntBuffer, eboIntBuffer: IntBuffer)
{
    glGenVertexArrays(1, vaoIntBuffer)
    val vao = vaoIntBuffer[0]
    glBindVertexArray(vao)

    glGenBuffers(1, // Num objects to generate
        vboIntBuffer)  // Out parameters to store IDs of gen objects
    val vbo = vboIntBuffer[0]
    glBindBuffer(GL_ARRAY_BUFFER, vbo) // bind object to array buffer
    glBufferData(GL_ARRAY_BUFFER, // which buffer data is being entered in
        cubePosTexNormAttributes.size * BYTES_PER_FLOAT, // size of data being placed in array buffer
        cubePosTexNormAttributes.toFloatBuffer(), // data to store in array buffer
        GL_STATIC_DRAW) // GL_STATIC_DRAW (most likely not change), GL_DYNAMIC_DRAW (likely to change), GL_STREAM_DRAW (changes every time drawn)

    // position attribute
    glVertexAttribPointer(0, // position vertex attribute (used for location = 0 of Vertex Shader)
        3, // size of vertex attribute (we're using vec3)
        GL_FLOAT, // type of data being passed
        false, // whether the data needs to be normalized
        cubePosTexNormAttSizeInBytes, // stride: space between consecutive vertex attribute sets
        0) // offset of where the data starts in the array
    glEnableVertexAttribArray(0)

    // normal attribute
    glVertexAttribPointer(1,
        3,
        GL_FLOAT,
        false,
        cubePosTexNormAttSizeInBytes,
        3 * BYTES_PER_FLOAT)
    glEnableVertexAttribArray(1)

    // texture coords attribute
    glVertexAttribPointer(2,
        2,
        GL_FLOAT,
        false,
        cubePosTexNormAttSizeInBytes,
        6 * BYTES_PER_FLOAT)
    glEnableVertexAttribArray(2)

    glGenBuffers(1, eboIntBuffer)
    val ebo = eboIntBuffer[0]
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        cubePosNormIndices.size * BYTES_PER_INT,
        cubePosNormIndices.toIntBuffer(),
        GL_STATIC_DRAW)

    // unbind VBO, VAO, & EBO
    glBindBuffer(GL_ARRAY_BUFFER, 0)
    glBindVertexArray(0)
    // Must unbind EBO AFTER unbinding VAO, since VAO stores all glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _) calls
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)
}


fun initializeFrameBufferQuadVertexAttBuffers(vaoIntBuffer: IntBuffer, vboIntBuffer: IntBuffer, eboIntBuffer: IntBuffer)
{
    glGenVertexArrays(1, vaoIntBuffer)
    glGenBuffers(1,
        vboIntBuffer)
    glGenBuffers(1, eboIntBuffer)

    val vao = vaoIntBuffer[0]
    val vbo = vboIntBuffer[0]
    val ebo = eboIntBuffer[0]

    glBindVertexArray(vao)

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        frameBufferQuadVertexAttributes.size * BYTES_PER_FLOAT,
        frameBufferQuadVertexAttributes.toFloatBuffer(),
        GL_STATIC_DRAW)

    // set the vertex attributes (position and texture)
    // position attribute
    glVertexAttribPointer(0,
        2,
        GL_FLOAT,
        false,
        frameBufferQuadVertexAttSizeInBytes,
        0)
    glEnableVertexAttribArray(0)

    // texture attribute
    glVertexAttribPointer(1,
        2,
        GL_FLOAT,
        false,
        frameBufferQuadVertexAttSizeInBytes,
        2 * BYTES_PER_FLOAT)
    glEnableVertexAttribArray(1)

    // bind element buffer object to give indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        quadIndices.size * BYTES_PER_INT,
        quadIndices.toIntBuffer(),
        GL_STATIC_DRAW)

    // unbind VBO, VAO, & EBO
    glBindBuffer(GL_ARRAY_BUFFER, 0)
    glBindVertexArray(0)
    // Must unbind EBO AFTER unbinding VAO, since VAO stores all glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _) calls
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)
}

data class FrameBuffer(val index: IntBuffer = IntBuffer.allocate(1),
                       val renderBufferIndex: IntBuffer = IntBuffer.allocate(1),
                       val textureBufferIndex: IntBuffer = IntBuffer.allocate(1))
fun FrameBuffer.delete() {
    glDeleteFramebuffers(1, index)
    glDeleteRenderbuffers(1, renderBufferIndex)
    glDeleteTextures(1, textureBufferIndex)
}
fun initializeFrameBuffer(frameBuffer: FrameBuffer, width: Int, height: Int) {
    // creating frame buffer
    glGenFramebuffers(1, frameBuffer.index)
    val frameBufferIndex = frameBuffer.index[0]
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIndex)

    // creating frame buffer texture
    glGenTextures(1, frameBuffer.textureBufferIndex)
    val frameBufferTextureIndex = frameBuffer.textureBufferIndex[0]
    glBindTexture(GL_TEXTURE_2D, frameBufferTextureIndex)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, null)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
    glBindTexture(GL_TEXTURE_2D, 0) // unbind

    // attach texture w/ color to frame buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, // frame buffer we're tageting (draw, read, or both)
        GL_COLOR_ATTACHMENT0, // type of attachment
        GL_TEXTURE_2D, // type of texture
        frameBufferTextureIndex, // texture
        0) // mipmap level

    // creating render buffer to be depth/stencil buffer
    glGenRenderbuffers(1, frameBuffer.renderBufferIndex)
    val rboIndex = frameBuffer.renderBufferIndex[0]
    glBindRenderbuffer(GL_RENDERBUFFER, rboIndex)
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height)
    glBindRenderbuffer(GL_RENDERBUFFER, 0) // unbind
    // attach render buffer w/ depth & stencil to frame buffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, // frame buffer target
        GL_DEPTH_STENCIL_ATTACHMENT, // attachment point of frame buffer
        GL_RENDERBUFFER, // render buffer target
        rboIndex)  // render buffer

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Log.e("ObjectData", "ERROR::FRAMEBUFFER:: Framebuffer is not complete!")
        throw RuntimeException("Error creating frame buffer")
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0)
}