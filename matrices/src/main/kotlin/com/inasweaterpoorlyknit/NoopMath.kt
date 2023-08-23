package com.inasweaterpoorlyknit

import kotlin.math.PI
import kotlin.math.cos
import kotlin.math.sin
import kotlin.math.sqrt
import kotlin.math.tan

/*
    TODO:
        - Rotate(startMat, radians, axis): Mat3
        - Perspective(radians, aspectRatio, zNear, zFar): Mat4
 */

const val radiansPerDegree = PI / 180.0

fun Double.degToRad(): Double = this * radiansPerDegree
fun Float.degToRad(): Float = this * radiansPerDegree.toFloat()

class Vec2 {
    val elements: FloatArray
    constructor(fillVal: Float = 0f){ elements = FloatArray(2){fillVal} }
    constructor(x: Float, y: Float) { elements = floatArrayOf(x, y) }

    val x get() = elements[0]
    val y get() = elements[1]
    val lenSq get() = (x*x) + (y*y)
    val len get() = sqrt(lenSq)
    val normalized get() = this / len
    fun dot(v: Vec2) = (x * v.x) + (y * v.y)

    operator fun unaryMinus() = Vec2(-x, -y)
    operator fun plus(v: Vec2) = Vec2(x+v.x, y+v.y)
    operator fun minus(v: Vec2) = Vec2(x-v.x, y-v.y)
    operator fun times(s: Float) = Vec2(x*s, y*s)
    operator fun div(s: Float) = this * (1f/s)
    operator fun times(m: Mat2) = Vec2( // Vec2 as row vector
        this.x * m.elements[0] + this.y * m.elements[2],
        this.x * m.elements[1] + this.y * m.elements[3]
    )

    override fun equals(other: Any?) = other is Vec2 && elements.contentEquals(other.elements)
    override fun hashCode() = elements.contentHashCode()
    override fun toString() = "Vec2(x=$x, y=$y)"
}

class Vec3 {
    val elements: FloatArray
    constructor(fillVal: Float = 0f){ elements = FloatArray(3){fillVal} }
    constructor(x: Float, y: Float, z: Float) { elements = floatArrayOf(x, y, z) }

    val x get() = elements[0]
    val y get() = elements[1]
    val z get() = elements[2]
    val r get() = elements[0]
    val g get() = elements[1]
    val b get() = elements[2]
    val lenSq get() = (x*x) + (y*y) + (z*z)
    val len get() = sqrt(lenSq)
    val normalized get() = this / len
    fun dot(v: Vec3) = (x * v.x) + (y * v.y) + (z * v.z)
    fun cross(v: Vec3) = Vec3(y * v.z - v.y * z,
                                z * v.x - v.z * x,
                                x * v.y - v.x * y)

    operator fun unaryMinus() = Vec3(-x, -y, -z)
    operator fun plus(v: Vec3) = Vec3(x+v.x, y+v.y, z+v.z)
    operator fun minus(v: Vec3) = Vec3(x-v.x, y-v.y, z-v.z)
    operator fun Float.times(v: Vec3) = Vec3(v.x*this, v.y*this, v.z*this)
    operator fun times(s: Float) = Vec3(x*s, y*s, z*s)
    operator fun div(s: Float) = this * (1f/s)
    operator fun rem(d: Float) = Vec3(x%d, y%d, z%d)
    operator fun times(m: Mat3) = Vec3( // Vec3 as row vector
        this.x * m.elements[0] + this.y * m.elements[3] + this.z * m.elements[6],
        this.x * m.elements[1] + this.y * m.elements[4] + this.z * m.elements[7],
        this.x * m.elements[2] + this.y * m.elements[5] + this.z * m.elements[8]
    )

    override fun equals(other: Any?) = other is Vec3 && elements.contentEquals(other.elements)
    override fun hashCode() = elements.contentHashCode()
    override fun toString() = "Vec3(x=$x, y=$y, z=$z)"
}

class Vec4 {
    val elements: FloatArray
    constructor(fillVal: Float = 0f){ elements = FloatArray(4){fillVal} }
    constructor(x: Float, y: Float, z: Float, w: Float) { elements = floatArrayOf(x, y, z, w) }

    val x get() = elements[0]
    val y get() = elements[1]
    val z get() = elements[2]
    val w get() = elements[3]
    val r get() = elements[0]
    val g get() = elements[1]
    val b get() = elements[2]
    val a get() = elements[3]
    val lenSq get() = (x*x) + (y*y) + (z*z) + (w*w)
    val len get() = sqrt(lenSq)
    val normalized get() = this / len
    fun dot(v: Vec4) = (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w)

    operator fun unaryMinus() = Vec4(-x, -y, -z, -w)
    operator fun plus(v: Vec4) = Vec4(x+v.x, y+v.y, z+v.z, w+v.w)
    operator fun minus(v: Vec4) = Vec4(x-v.x, y-v.y, z-v.z, w-v.w)
    operator fun Float.times(v: Vec4) = Vec4(v.x*this, v.y*this, v.z*this, v.w*this)
    operator fun times(s: Float) = Vec4(x*s, y*s, z*s, w*s)
    operator fun div(s: Float) = this * (1f/s)
    operator fun times(m: Mat4) = Vec4( // Vec4 as row vector
        this.x * m.elements[0] + this.y * m.elements[4] + this.z * m.elements[8] + this.w * m.elements[12],
        this.x * m.elements[1] + this.y * m.elements[5] + this.z * m.elements[9] + this.w * m.elements[13],
        this.x * m.elements[2] + this.y * m.elements[6] + this.z * m.elements[10] + this.w * m.elements[14],
        this.x * m.elements[3] + this.y * m.elements[7] + this.z * m.elements[11] + this.w * m.elements[15]
    )

    override fun equals(other: Any?) = other is Vec4 && elements.contentEquals(other.elements)
    override fun hashCode() = elements.contentHashCode()
    override fun toString() = "Vec4(x=$x, y=$y, z=$z, w=$w)"

}

class Mat2 {
    val elements: FloatArray
    val col0 get() = Vec2(elements[0], elements[2])
    val col1 get() = Vec2(elements[1], elements[3])
    val row0 get() = Vec2(elements[0], elements[1])
    val row1 get() = Vec2(elements[2], elements[3])

    constructor() { elements = FloatArray(2*2){0f} }
    constructor(diagonal: Float) {
        elements = floatArrayOf(diagonal, 0f,
                                0f, diagonal)
    }
    constructor(e00: Float, e01: Float,
                e10: Float, e11: Float,) {
        elements = floatArrayOf(e00, e01,
                                e10, e11,)
    }

    operator fun times(other: Mat2) = Mat2 (
        (this.elements[0] * other.elements[0]) + (this.elements[1] * other.elements[2]),
        (this.elements[0] * other.elements[1]) + (this.elements[1] * other.elements[3]),
        (this.elements[2] * other.elements[0]) + (this.elements[3] * other.elements[2]),
        (this.elements[2] * other.elements[1]) + (this.elements[3] * other.elements[3])
    )

    // Vec2 as column vector
    operator fun times(v: Vec2) = Vec2(
        v.x * this.elements[0] + v.y * this.elements[1],
        v.x * this.elements[2] + v.y * this.elements[3]
    )

    override fun equals(other: Any?) = other is Mat2 && this.elements.contentEquals(other.elements)
    override fun hashCode() = elements.contentHashCode()
    override fun toString() = "Mat2(${elements.contentToString()})"
}

class Mat3 {
    val elements: FloatArray
    val col0 get() = Vec3(elements[0], elements[3], elements[6])
    val col1 get() = Vec3(elements[1], elements[4], elements[7])
    val col2 get() = Vec3(elements[2], elements[5], elements[8])
    val row0 get() = Vec3(elements[0], elements[1], elements[2])
    val row1 get() = Vec3(elements[3], elements[4], elements[5])
    val row2 get() = Vec3(elements[6], elements[7], elements[8])

    constructor() { elements = FloatArray(3*3){0f} }
    constructor(diagonal: Float) {
        elements = floatArrayOf(diagonal, 0f, 0f,
                                0f, diagonal, 0f,
                                0f, 0f, diagonal)
    }

    constructor(e00: Float, e01: Float, e02: Float,
                e10: Float, e11: Float, e12: Float,
                e20: Float, e21: Float, e22: Float) {
        elements = floatArrayOf(e00, e01, e02,
                                e10, e11, e12,
                                e20, e21, e22)
    }

    operator fun times(other: Mat3) = Mat3 (
        (this.elements[0] * other.elements[0]) + (this.elements[1] * other.elements[3]) + (this.elements[2] * other.elements[6]),
        (this.elements[0] * other.elements[1]) + (this.elements[1] * other.elements[4]) + (this.elements[2] * other.elements[7]),
        (this.elements[0] * other.elements[2]) + (this.elements[1] * other.elements[5]) + (this.elements[2] * other.elements[8]),
        (this.elements[3] * other.elements[0]) + (this.elements[4] * other.elements[3]) + (this.elements[5] * other.elements[6]),
        (this.elements[3] * other.elements[1]) + (this.elements[4] * other.elements[4]) + (this.elements[5] * other.elements[7]),
        (this.elements[3] * other.elements[2]) + (this.elements[4] * other.elements[5]) + (this.elements[5] * other.elements[8]),
        (this.elements[6] * other.elements[0]) + (this.elements[7] * other.elements[3]) + (this.elements[8] * other.elements[6]),
        (this.elements[6] * other.elements[1]) + (this.elements[7] * other.elements[4]) + (this.elements[8] * other.elements[7]),
        (this.elements[6] * other.elements[2]) + (this.elements[7] * other.elements[5]) + (this.elements[8] * other.elements[8])
    )

    operator fun times(other: Mat4) = Mat4(
        (this.elements[0] * other.elements[0]) + (this.elements[1] * other.elements[4]) + (this.elements[2] * other.elements[8]),
        (this.elements[0] * other.elements[1]) + (this.elements[1] * other.elements[5]) + (this.elements[2] * other.elements[9]),
        (this.elements[0] * other.elements[2]) + (this.elements[1] * other.elements[6]) + (this.elements[2] * other.elements[10]),
        (this.elements[0] * other.elements[3]) + (this.elements[1] * other.elements[7]) + (this.elements[2] * other.elements[11]),
        (this.elements[3] * other.elements[0]) + (this.elements[4] * other.elements[4]) + (this.elements[5] * other.elements[8]),
        (this.elements[3] * other.elements[1]) + (this.elements[4] * other.elements[5]) + (this.elements[5] * other.elements[9]),
        (this.elements[3] * other.elements[2]) + (this.elements[4] * other.elements[6]) + (this.elements[5] * other.elements[10]),
        (this.elements[3] * other.elements[3]) + (this.elements[4] * other.elements[7]) + (this.elements[5] * other.elements[11]),
        (this.elements[6] * other.elements[0]) + (this.elements[7] * other.elements[4]) + (this.elements[8] * other.elements[8]),
        (this.elements[6] * other.elements[1]) + (this.elements[7] * other.elements[5]) + (this.elements[8] * other.elements[9]),
        (this.elements[6] * other.elements[2]) + (this.elements[7] * other.elements[6]) + (this.elements[8] * other.elements[10]),
        (this.elements[6] * other.elements[3]) + (this.elements[7] * other.elements[7]) + (this.elements[8] * other.elements[11]),
        other.elements[12],
        other.elements[13],
        other.elements[14],
        other.elements[15]
    )

    // Vec3 as column vector
    operator fun times(v: Vec3) = Vec3(
        v.x * this.elements[0] + v.y * this.elements[1] + v.z * this.elements[2],
        v.x * this.elements[3] + v.y * this.elements[4] + v.z * this.elements[5],
        v.x * this.elements[6] + v.y * this.elements[7] + v.z * this.elements[8]
    )

    companion object {
        // TODO: This method may be messed up due to using code from column major matrices
        fun rotate(radians: Double, v: Vec3): Mat3 {
                val axis = v.normalized

                val cosA = cos(radians).toFloat()
                val sinA = sin(radians).toFloat()
                val axisTimesOneMinusCos = axis * (1f - cosA)

                return Mat3(axis.x * axisTimesOneMinusCos.x + cosA,
                        axis.x * axisTimesOneMinusCos.y + sinA * axis.z,
                        axis.x * axisTimesOneMinusCos.z - sinA * axis.y,

                        axis.y * axisTimesOneMinusCos.x - sinA * axis.z,
                        axis.y * axisTimesOneMinusCos.y + cosA,
                        axis.y * axisTimesOneMinusCos.z + sinA * axis.x,

                        axis.z * axisTimesOneMinusCos.x + sinA * axis.y,
                        axis.z * axisTimesOneMinusCos.y - sinA * axis.x,
                        axis.z * axisTimesOneMinusCos.z + cosA)
        }
    }

    override fun equals(other: Any?) = other is Mat3 && this.elements.contentEquals(other.elements)
    override fun hashCode() = elements.contentHashCode()
    override fun toString() = "Mat3(${elements.contentToString()})"
}

class Mat4 {
    val elements: FloatArray
    val col0 get() = Vec4(elements[0], elements[4], elements[8], elements[12])
    val col1 get() = Vec4(elements[1], elements[5], elements[9], elements[13])
    val col2 get() = Vec4(elements[2], elements[6], elements[10], elements[14])
    val col3 get() = Vec4(elements[3], elements[7], elements[11], elements[15])
    val row0 get() = Vec4(elements[0], elements[1], elements[2], elements[3])
    val row1 get() = Vec4(elements[4], elements[5], elements[6], elements[7])
    val row2 get() = Vec4(elements[8], elements[9], elements[10], elements[11])
    val row3 get() = Vec4(elements[12], elements[13], elements[14], elements[15])

    constructor(){ elements = FloatArray(4*4){0f} }
    constructor(diagonal: Float) {
        elements = floatArrayOf(diagonal, 0f, 0f, 0f,
                                0f, diagonal, 0f, 0f,
                                0f, 0f, diagonal, 0f,
                                0f, 0f, 0f, diagonal)
    }
    constructor(e00: Float, e01: Float, e02: Float, e03: Float,
                e10: Float, e11: Float, e12: Float, e13: Float,
                e20: Float, e21: Float, e22: Float, e23: Float,
                e30: Float, e31: Float, e32: Float, e33: Float) {
        elements = floatArrayOf(e00, e01, e02, e03,
                                e10, e11, e12, e13,
                                e20, e21, e22, e23,
                                e30, e31, e32, e33)
    }
    constructor(mat3: Mat3) {
        elements = floatArrayOf(mat3.elements[0], mat3.elements[1], mat3.elements[2], 0f,
                                mat3.elements[3], mat3.elements[4], mat3.elements[5], 0f,
                                mat3.elements[6], mat3.elements[7], mat3.elements[8], 0f,
                                              0f,               0f,               0f, 1f)
    }

    operator fun times(other: Mat4) = Mat4(
        (this.elements[0] * other.elements[0]) + (this.elements[1] * other.elements[4]) + (this.elements[2] * other.elements[8]) + (this.elements[3] * other.elements[12]),
        (this.elements[0] * other.elements[1]) + (this.elements[1] * other.elements[5]) + (this.elements[2] * other.elements[9]) + (this.elements[3] * other.elements[13]),
        (this.elements[0] * other.elements[2]) + (this.elements[1] * other.elements[6]) + (this.elements[2] * other.elements[10]) + (this.elements[3] * other.elements[14]),
        (this.elements[0] * other.elements[3]) + (this.elements[1] * other.elements[7]) + (this.elements[2] * other.elements[11]) + (this.elements[3] * other.elements[15]),
        (this.elements[4] * other.elements[0]) + (this.elements[5] * other.elements[4]) + (this.elements[6] * other.elements[8]) + (this.elements[7] * other.elements[12]),
        (this.elements[4] * other.elements[1]) + (this.elements[5] * other.elements[5]) + (this.elements[6] * other.elements[9]) + (this.elements[7] * other.elements[13]),
        (this.elements[4] * other.elements[2]) + (this.elements[5] * other.elements[6]) + (this.elements[6] * other.elements[10]) + (this.elements[7] * other.elements[14]),
        (this.elements[4] * other.elements[3]) + (this.elements[5] * other.elements[7]) + (this.elements[6] * other.elements[11]) + (this.elements[7] * other.elements[15]),
        (this.elements[8] * other.elements[0]) + (this.elements[9] * other.elements[4]) + (this.elements[10] * other.elements[8]) + (this.elements[11] * other.elements[12]),
        (this.elements[8] * other.elements[1]) + (this.elements[9] * other.elements[5]) + (this.elements[10] * other.elements[9]) + (this.elements[11] * other.elements[13]),
        (this.elements[8] * other.elements[2]) + (this.elements[9] * other.elements[6]) + (this.elements[10] * other.elements[10]) + (this.elements[11] * other.elements[14]),
        (this.elements[8] * other.elements[3]) + (this.elements[9] * other.elements[7]) + (this.elements[10] * other.elements[11]) + (this.elements[11] * other.elements[15]),
        (this.elements[12] * other.elements[0]) + (this.elements[13] * other.elements[4]) + (this.elements[14] * other.elements[8]) + (this.elements[15] * other.elements[12]),
        (this.elements[12] * other.elements[1]) + (this.elements[13] * other.elements[5]) + (this.elements[14] * other.elements[9]) + (this.elements[15] * other.elements[13]),
        (this.elements[12] * other.elements[2]) + (this.elements[13] * other.elements[6]) + (this.elements[14] * other.elements[10]) + (this.elements[15] * other.elements[14]),
        (this.elements[12] * other.elements[3]) + (this.elements[13] * other.elements[7]) + (this.elements[14] * other.elements[11]) + (this.elements[15] * other.elements[15])
    )

    operator fun times(other: Mat3) = Mat4(
        (this.elements[0] * other.elements[0]) + (this.elements[1] * other.elements[3]) + (this.elements[2] * other.elements[6]),
        (this.elements[0] * other.elements[1]) + (this.elements[1] * other.elements[4]) + (this.elements[2] * other.elements[7]),
        (this.elements[0] * other.elements[2]) + (this.elements[1] * other.elements[5]) + (this.elements[2] * other.elements[8]),
        this.elements[3],
        (this.elements[4] * other.elements[0]) + (this.elements[5] * other.elements[3]) + (this.elements[6] * other.elements[6]),
        (this.elements[4] * other.elements[1]) + (this.elements[5] * other.elements[4]) + (this.elements[6] * other.elements[7]),
        (this.elements[4] * other.elements[2]) + (this.elements[5] * other.elements[5]) + (this.elements[6] * other.elements[8]),
        this.elements[7],
        (this.elements[8] * other.elements[0]) + (this.elements[9] * other.elements[3]) + (this.elements[10] * other.elements[6]),
        (this.elements[8] * other.elements[1]) + (this.elements[9] * other.elements[4]) + (this.elements[10] * other.elements[7]),
        (this.elements[8] * other.elements[2]) + (this.elements[9] * other.elements[5]) + (this.elements[10] * other.elements[8]),
        this.elements[11],
        (this.elements[12] * other.elements[0]) + (this.elements[13] * other.elements[3]) + (this.elements[14] * other.elements[6]),
        (this.elements[12] * other.elements[1]) + (this.elements[13] * other.elements[4]) + (this.elements[14] * other.elements[7]),
        (this.elements[12] * other.elements[2]) + (this.elements[13] * other.elements[5]) + (this.elements[14] * other.elements[8]),
        this.elements[15]
    )

    // Vec4 as column vector
    operator fun times(v: Vec4) = Vec4(
        v.x * this.elements[0] + v.y * this.elements[1] + v.z * this.elements[2] + v.w * this.elements[3],
        v.x * this.elements[4] + v.y * this.elements[5] + v.z * this.elements[6] + v.w * this.elements[7],
        v.x * this.elements[8] + v.y * this.elements[9] + v.z * this.elements[10] + v.w * this.elements[11],
        v.x * this.elements[12] + v.y * this.elements[13] + v.z * this.elements[14] + v.w * this.elements[15]
    )

    companion object {
        // TODO: This method may be messed up due to using code from column major matrices
        // real-time rendering 4.7.2
        // aspect ratio is equivalent to width / height
        fun perspective(fovVert: Double, aspect: Double, near: Double, far: Double): Mat4 {
            val c = 1.0 / tan(fovVert / 2.0)
            return Mat4(
                (c / aspect).toFloat(), 0f, 0f, 0f,
                0f, c.toFloat(), 0f, 0f,
                0f, 0f, (-(far + near) / (far - near)).toFloat(), -1f,
                0f, 0f, (-(2.0 * far * near) / (far - near)).toFloat(), 0f,
            )
        }
    }

    override fun equals(other: Any?) = other is Mat4 && this.elements.contentEquals(other.elements)
    override fun hashCode() = elements.contentHashCode()
    override fun toString() = "Mat4(${elements.contentToString()})"
}