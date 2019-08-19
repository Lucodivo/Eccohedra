package com.inasweaterpoorlyknit.learnopengl_androidport.utils

fun systemTimeInSeconds() : Float {
    return System.nanoTime().toFloat() / 1000000000.0f
}