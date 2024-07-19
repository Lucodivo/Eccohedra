package com.inasweaterpoorlyknit.scenes.graphics

import android.content.Context
import android.content.res.Configuration.ORIENTATION_LANDSCAPE
import android.content.res.Resources
import android.util.Log
import android.view.Surface
import android.view.WindowManager
import androidx.annotation.RawRes
import com.inasweaterpoorlyknit.noopmath.Mat2
import com.inasweaterpoorlyknit.noopmath.Vec2
import kotlin.math.atan2
import kotlin.math.cos
import kotlin.math.sin

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

fun getResourceRawFileAsString(resources: Resources, @RawRes id: Int): String {
  val rawResInputStream = resources.openRawResource(id)
  val rawResAsString = java.util.Scanner(rawResInputStream).useDelimiter("\\A")
  val result = if (rawResAsString.hasNext()) rawResAsString.next() else ""
  rawResInputStream.close()
  return result
}

fun Orientation.isLandscape(): Boolean = this == Orientation.Landscape || this == Orientation.LandscapeReverse

val Context.orientation: Orientation
  get() {
    val rotation = if (android.os.Build.VERSION.SDK_INT >= 30) {
      display?.rotation
    } else { // TODO: WindowManager.defaultDisplay is deprecated, remove when minSDK is 30+
      val windowManager = getSystemService(Context.WINDOW_SERVICE) as WindowManager?
      windowManager?.defaultDisplay?.rotation
    }

    return if (rotation != null) {
      if (resources.configuration.orientation == ORIENTATION_LANDSCAPE) {
        if (rotation == Surface.ROTATION_90 || rotation == Surface.ROTATION_0) {
          Orientation.Landscape
        } else {
          Orientation.LandscapeReverse
        }
      } else if (rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_270) {
        Orientation.Portrait
      } else {
        Orientation.PortraitReverse
      }
    } else {
      Log.e("Orientation Error", "This context does not have access to the display.")
      Orientation.Portrait
    }
  }

fun rotationMat2D(radians: Float): Mat2 { // CCW
  val cosRad = cos(radians)
  val sinRad = sin(radians)
  return Mat2(
    cosRad, -sinRad,
    sinRad, cosRad
  )
}

// Found on stack overflow. Determine the angle between two lines.
// Question: Android Two finger rotation by paulot
// Answer Author: leszek.hanusz
// src: https://stackoverflow.com/questions/10682019/android-two-finger-rotation
fun angleBetweenLines(
  pointA1: Vec2, pointA2: Vec2, // line 1
  pointB1: Vec2, pointB2: Vec2  // line 2
): Float {
  val angle1 = atan2((pointA2.y - pointA1.y).toDouble(), (pointA2.x - pointA1.x).toDouble())
  val angle2 = atan2((pointB2.y - pointB1.y).toDouble(), (pointB2.x - pointB1.x).toDouble())
  var angle = (angle1 - angle2) % (TWO_PI)
  if (angle < -PI) angle += TWO_PI
  if (angle > PI) angle -= TWO_PI
  return angle.toFloat()
}