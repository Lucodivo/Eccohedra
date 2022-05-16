package com.inasweaterpoorlyknit.learnopengl_androidport.graphics

import android.content.Context
import androidx.annotation.RawRes

fun systemTimeInSeconds(): Double {
  return System.nanoTime().toDouble() / 1000000000
}

fun systemTimeInDeciseconds(): Double {
  // note: time measured in deciseconds (10^-1 seconds)
  return System.nanoTime().toDouble() / 100000000
}

fun getResourceRawFileAsString(context: Context, @RawRes id: Int): String {
  val rawResInputStream = context.resources.openRawResource(id)
  val rawResAsString = java.util.Scanner(rawResInputStream).useDelimiter("\\A")
  val result = if (rawResAsString.hasNext()) rawResAsString.next() else ""
  rawResInputStream.close()
  return result
}

val Context.orientation: Int
  get() = resources.configuration.orientation