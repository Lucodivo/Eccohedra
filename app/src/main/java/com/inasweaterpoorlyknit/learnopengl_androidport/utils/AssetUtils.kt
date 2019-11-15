package com.inasweaterpoorlyknit.learnopengl_androidport.utils

import android.content.Context
import androidx.annotation.RawRes
import java.io.InputStream

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