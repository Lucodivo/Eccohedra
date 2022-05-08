package com.inasweaterpoorlyknit.learnopengl_androidport.utils

import android.view.View

fun systemTimeInSeconds(): Double {
  return System.nanoTime().toDouble() / 1000000000
}

fun systemTimeInDeciseconds(): Double {
  // note: time measured in deciseconds (10^-1 seconds)
  return System.nanoTime().toDouble() / 100000000
}

fun androidx.appcompat.app.AppCompatActivity.hideSystemUI() {
  window.decorView.systemUiVisibility = (
      View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or // hide the navigation
      View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or // lay out view as if the navigation will be hidden
      View.SYSTEM_UI_FLAG_IMMERSIVE or // used with HIDE_NAVIGATION to remain interactive when hiding navigation
      View.SYSTEM_UI_FLAG_FULLSCREEN or // fullscreen
      View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or // lay out view as if fullscreen
      View.SYSTEM_UI_FLAG_LAYOUT_STABLE) // stable view of content (layout view size doesn't change)
}