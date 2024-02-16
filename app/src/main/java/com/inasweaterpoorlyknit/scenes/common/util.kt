package com.inasweaterpoorlyknit.scenes.common

fun clamp(value: Int, min: Int, max: Int): Int = if(value < min) min else if(value > max) max else value
