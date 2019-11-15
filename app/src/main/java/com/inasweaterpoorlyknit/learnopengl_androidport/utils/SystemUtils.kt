package com.inasweaterpoorlyknit.learnopengl_androidport.utils

fun systemTimeInSeconds() : Float {
    return System.nanoTime().toFloat() / 1000000000.0f
}

fun systemTimeInDeciseconds() : Float {
    // note: time measured in deciseconds (10^-1 seconds)
    return System.nanoTime().toFloat() / 100000000.0f
}