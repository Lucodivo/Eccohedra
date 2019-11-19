package com.inasweaterpoorlyknit.learnopengl_androidport.utils

fun systemTimeInSeconds() : Double {
    return System.nanoTime().toDouble() / 1000000000
}

fun systemTimeInDeciseconds() : Double {
    // note: time measured in deciseconds (10^-1 seconds)
    return System.nanoTime().toDouble() / 100000000
}