package com.inasweaterpoorlyknit.learnopengl_androidport

import kotlin.math.sqrt

data class Vec3(val x: Float = 0.0f, val y: Float = 0.0f, val z: Float = 0.0f)

operator fun Vec3.unaryMinus() = Vec3(-x, -y, -z)
operator fun Vec3.plus(second: Vec3) = Vec3(x + second.x, y + second.y, z + second.z)
operator fun Vec3.minus(second: Vec3) = Vec3(x - second.x, y - second.y, z - second.z)

fun Vec3.length() = sqrt((x*x) + (y*y) + (z*z))
fun Vec3.normalize(): Vec3 {
    val length = length()
    return Vec3(x/length, y/length, z/length)
}

fun cross(first: Vec3, second: Vec3): Vec3 {
    return Vec3(first.y * second.z - first.z * second.y,
                first.z * second.x - first.x * second.z,
                first.x * second.y - first.y * second.x)
}