package com.inasweaterpoorlyknit.learnopengl_androidport

import android.content.Context
import androidx.annotation.RawRes

fun getResourceRawFileAsString(context: Context, @RawRes id: Int): String {
    val rawResInputStream = context.resources.openRawResource(id)
    val rawResAsString = java.util.Scanner(rawResInputStream).useDelimiter("\\A")
    val result = if (rawResAsString.hasNext()) rawResAsString.next() else ""
    rawResInputStream.close()
    return result
}