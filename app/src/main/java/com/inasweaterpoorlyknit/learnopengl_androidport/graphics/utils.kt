package com.inasweaterpoorlyknit.learnopengl_androidport.graphics

import android.content.Context
import android.content.res.Configuration.ORIENTATION_LANDSCAPE
import android.view.Surface
import android.view.WindowManager
import androidx.annotation.RawRes

enum class Orientation {
  Portrait,
  Landscape,
  PortraitReverse, // Not supported in the app
  LandscapeReverse
}

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

fun Orientation.isLandscape(): Boolean = this == Orientation.Landscape || this == Orientation.LandscapeReverse

val Context.orientation: Orientation
  get() {
    val windowManager = getSystemService(Context.WINDOW_SERVICE) as WindowManager
    val rotation = windowManager.defaultDisplay.rotation
    return if(resources.configuration.orientation == ORIENTATION_LANDSCAPE) {
      if(rotation == Surface.ROTATION_90 || rotation == Surface.ROTATION_0) {
        Orientation.Landscape
      } else {
        Orientation.LandscapeReverse
      }
    } else if(rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_270) {
      Orientation.Portrait
    } else {
      Orientation.PortraitReverse
    }
  }