package com.inasweaterpoorlyknit

import kotlin.math.PI
import kotlin.math.cos
import kotlin.math.sin
import kotlin.math.sqrt
import kotlin.math.tan

/*
    TODO:
        - Investigate if operator overloading `operator fun get(i: Int) = elements[i]` has much of a performance penalty due to function overhead
 */

// NOTE: Matrices are stored column-major

const val radiansPerDegree = PI / 180.0

fun Double.degToRad(): Double = this * radiansPerDegree
fun Float.degToRad(): Float = this * radiansPerDegree.toFloat()

fun clamp(min: Double, max: Double, value: Double) = if (value < min) { min } else if (value > max) { max } else { value }
fun lerp(x: Float, y: Float, a: Float) = x * (1.0f - a) + (y * a)

// Just using Khronos' definition for smoothstep in GLSL
// https://registry.khronos.org/OpenGL-Refpages/gl4/html/smoothstep.xhtml
fun smoothStep(edge0: Double, edge1: Double, x: Double): Double {
    val t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t)
}

class Vec2 {
    val elements: FloatArray
    constructor(fillVal: Float = 0f){ elements = FloatArray(2){fillVal} }
    constructor(x: Float, y: Float) { elements = floatArrayOf(x, y) }

    val x inline get() = elements[0]
    val y inline get() = elements[1]
    val lenSq inline get() = (x*x) + (y*y)
    val len inline get() = sqrt(lenSq)
    val normalized inline get() = this / len
    fun dot(v: Vec2) = (x * v.x) + (y * v.y)

    operator fun get(i: Int) = elements[i]
    operator fun unaryMinus() = Vec2(-x, -y)
    operator fun plus(v: Vec2) = Vec2(x+v.x, y+v.y)
    operator fun minus(v: Vec2) = Vec2(x-v.x, y-v.y)
    operator fun times(s: Float) = Vec2(x*s, y*s)
    operator fun div(s: Float) = this * (1f/s)
    operator fun times(m: Mat2) = Vec2( // Vec2 treated as row vector
        this.x * m[0] + this.y * m[1],
        this.x * m[2] + this.y * m[3]
    )

    override fun equals(other: Any?) = other is Vec2 && elements.contentEquals(other.elements)
    override fun hashCode() = elements.contentHashCode()
    override fun toString() = "Vec2(x=$x, y=$y)"
}

data class dVec2(val x: Double, val y: Double) {
    constructor(v: Vec2): this(v.x.toDouble(), v.y.toDouble())
    operator fun minus(v: dVec2) = dVec2(x-v.x, y-v.y)
    operator fun times(s: Double) = dVec2(x*s, y*s)
    fun toVec2() = Vec2(x.toFloat(), y.toFloat())
}

class Vec3 {
    val elements: FloatArray
    constructor(fillVal: Float = 0f){ elements = FloatArray(3){fillVal} }
    constructor(x: Float, y: Float, z: Float) { elements = floatArrayOf(x, y, z) }

    val x inline get() = elements[0]
    val y inline get() = elements[1]
    val z inline get() = elements[2]
    val r inline get() = elements[0]
    val g inline get() = elements[1]
    val b inline get() = elements[2]
    val lenSq inline get() = (x*x) + (y*y) + (z*z)
    val len inline get() = sqrt(lenSq)
    val normalized inline get() = this / len
    fun dot(v: Vec3) = (x * v.x) + (y * v.y) + (z * v.z)
    fun cross(v: Vec3) = Vec3(y * v.z - v.y * z,
                            z * v.x - v.z * x,
                            x * v.y - v.x * y)

    operator fun get(i: Int) = elements[i]
    operator fun unaryMinus() = Vec3(-x, -y, -z)
    operator fun plus(v: Vec3) = Vec3(x+v.x, y+v.y, z+v.z)
    operator fun minus(v: Vec3) = Vec3(x-v.x, y-v.y, z-v.z)
    operator fun Float.times(v: Vec3) = Vec3(v.x*this, v.y*this, v.z*this)
    operator fun times(s: Float) = Vec3(x*s, y*s, z*s)
    operator fun div(s: Float) = this * (1f/s)
    operator fun rem(d: Float) = Vec3(x%d, y%d, z%d)
    operator fun times(m: Mat3) = Vec3( // Vec3 treated as row vector
        this.x * m[0] + this.y * m[1] + this.z * m[2],
        this.x * m[3] + this.y * m[4] + this.z * m[5],
        this.x * m[6] + this.y * m[7] + this.z * m[8]
    )

    override fun equals(other: Any?) = other is Vec3 && elements.contentEquals(other.elements)
    override fun hashCode() = elements.contentHashCode()
    override fun toString() = "Vec3(x=$x, y=$y, z=$z)"
}

fun lerp(v1: Vec3, v2: Vec3, a: Float) = Vec3(lerp(v1.x, v2.x, a), lerp(v1.y, v2.y, a), lerp(v1.z, v2.z, a))

class Vec4 {
    val elements: FloatArray
    constructor(fillVal: Float = 0f){ elements = FloatArray(4){fillVal} }
    constructor(x: Float, y: Float, z: Float, w: Float) { elements = floatArrayOf(x, y, z, w) }

    val x inline get() = elements[0]
    val y inline get() = elements[1]
    val z inline get() = elements[2]
    val w inline get() = elements[3]
    val r inline get() = elements[0]
    val g inline get() = elements[1]
    val b inline get() = elements[2]
    val a inline get() = elements[3]
    val lenSq inline get() = (x*x) + (y*y) + (z*z) + (w*w)
    val len inline get() = sqrt(lenSq)
    val normalized inline get() = this / len
    fun dot(v: Vec4) = (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w)

    operator fun get(i: Int) = elements[i]
    operator fun unaryMinus() = Vec4(-x, -y, -z, -w)
    operator fun plus(v: Vec4) = Vec4(x+v.x, y+v.y, z+v.z, w+v.w)
    operator fun minus(v: Vec4) = Vec4(x-v.x, y-v.y, z-v.z, w-v.w)
    operator fun Float.times(v: Vec4) = Vec4(v.x*this, v.y*this, v.z*this, v.w*this)
    operator fun times(s: Float) = Vec4(x*s, y*s, z*s, w*s)
    operator fun div(s: Float) = this * (1f/s)
    operator fun times(m: Mat4) = Vec4( // Vec4 treated as row vector
        this.x * m[0] + this.y * m[1] + this.z * m[2] + this.w * m[3],
        this.x * m[4] + this.y * m[5] + this.z * m[6] + this.w * m[7],
        this.x * m[8] + this.y * m[9] + this.z * m[10] + this.w * m[11],
        this.x * m[12] + this.y * m[13] + this.z * m[14] + this.w * m[15]
    )

    override fun equals(other: Any?) = other is Vec4 && elements.contentEquals(other.elements)
    override fun hashCode() = elements.contentHashCode()
    override fun toString() = "Vec4(x=$x, y=$y, z=$z, w=$w)"
}

class Mat2 {
    val elements: FloatArray
    val col0 inline get() = Vec2(elements[0], elements[1])
    val col1 inline get() = Vec2(elements[2], elements[3])
    val row0 inline get() = Vec2(elements[0], elements[2])
    val row1 inline get() = Vec2(elements[1], elements[3])

    constructor() { elements = FloatArray(2*2){0f} }

    constructor(diagonal: Float) {
        elements = floatArrayOf(diagonal, 0f,
            0f, diagonal)
    }

    // NOTE: Column-major ordering
    constructor(e00: Float, e01: Float,
                e10: Float, e11: Float,) {
        elements = floatArrayOf(e00, e01,
            e10, e11)
    }

    operator fun get(i: Int) = elements[i]

    operator fun times(other: Mat2) = Mat2 (
        (this[0] * other[0]) + (this[2] * other[1]),
        (this[1] * other[0]) + (this[3] * other[1]),
        (this[0] * other[2]) + (this[2] * other[3]),
        (this[1] * other[2]) + (this[3] * other[3])
    )

    operator fun times(v: Vec2) = Vec2( // Vec2 treated as column vector
        this[0]*v[0] + this[2]*v[1],
        this[1]*v[0] + this[3]*v[1]
    )

    override fun equals(other: Any?) = other is Mat2 && this.elements.contentEquals(other.elements)
    override fun hashCode() = elements.contentHashCode()
    override fun toString() = "Mat2(${elements.contentToString()})"
}

class Mat3 {
    val elements: FloatArray
    val col0 inline get() = Vec3(elements[0], elements[1], elements[2])
    val col1 inline get() = Vec3(elements[3], elements[4], elements[5])
    val col2 inline get() = Vec3(elements[6], elements[7], elements[8])
    val row0 inline get() = Vec3(elements[0], elements[3], elements[6])
    val row1 inline get() = Vec3(elements[1], elements[4], elements[7])
    val row2 inline get() = Vec3(elements[2], elements[5], elements[8])

    constructor() { elements = FloatArray(3*3){0f} }

    constructor(diagonal: Float) {
        elements = floatArrayOf(diagonal, 0f, 0f,
            0f, diagonal, 0f,
            0f, 0f, diagonal)
    }

    // NOTE: Column-major ordering
    constructor(e00: Float, e01: Float, e02: Float,
                e10: Float, e11: Float, e12: Float,
                e20: Float, e21: Float, e22: Float) {
        elements = floatArrayOf(e00, e01, e02,
            e10, e11, e12,
            e20, e21, e22)
    }

    operator fun get(i: Int) = elements[i]

    operator fun times(other: Mat3) = Mat3 (
        (this[0] * other[0]) + (this[3] * other[1]) + (this[6] * other[2]),
        (this[1] * other[0]) + (this[4] * other[1]) + (this[7] * other[2]),
        (this[2] * other[0]) + (this[5] * other[1]) + (this[8] * other[2]),
        (this[0] * other[3]) + (this[3] * other[4]) + (this[6] * other[5]),
        (this[1] * other[3]) + (this[4] * other[4]) + (this[7] * other[5]),
        (this[2] * other[3]) + (this[5] * other[4]) + (this[8] * other[5]),
        (this[0] * other[6]) + (this[3] * other[7]) + (this[6] * other[8]),
        (this[1] * other[6]) + (this[4] * other[7]) + (this[7] * other[8]),
        (this[2] * other[6]) + (this[5] * other[7]) + (this[8] * other[8])
    )

    operator fun times(other: Mat4) = Mat4(
        (this[0] * other[0]) + (this[3] * other[1]) + (this[6] * other[2]),
        (this[1] * other[0]) + (this[4] * other[1]) + (this[7] * other[2]),
        (this[2] * other[0]) + (this[5] * other[1]) + (this[8] * other[2]),
        other[3],
        (this[0] * other[4]) + (this[3] * other[5]) + (this[6] * other[6]),
        (this[1] * other[4]) + (this[4] * other[5]) + (this[7] * other[6]),
        (this[2] * other[4]) + (this[5] * other[5]) + (this[8] * other[6]),
        other[7],
        (this[0] * other[8]) + (this[3] * other[9]) + (this[6] * other[10]),
        (this[1] * other[8]) + (this[4] * other[9]) + (this[7] * other[10]),
        (this[2] * other[8]) + (this[5] * other[9]) + (this[8] * other[10]),
        other[11],
        (this[0] * other[12]) + (this[3] * other[13]) + (this[6] * other[14]),
        (this[1] * other[12]) + (this[4] * other[13]) + (this[7] * other[14]),
        (this[2] * other[12]) + (this[5] * other[13]) + (this[8] * other[14]),
        other[15]
    )

    operator fun times(v: Vec3) = Vec3( // Vec3 treated as column vector
        this[0]*v[0] + this[3]*v[1] + this[6]*v[2],
        this[1]*v[0] + this[4]*v[1] + this[7]*v[2],
        this[2]*v[0] + this[5]*v[1] + this[8]*v[2]
    )

    companion object {
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
    val col0 inline get() = Vec4(elements[0], elements[1], elements[2], elements[3])
    val col1 inline get() = Vec4(elements[4], elements[5], elements[6], elements[7])
    val col2 inline get() = Vec4(elements[8], elements[9], elements[10], elements[11])
    val col3 inline get() = Vec4(elements[12], elements[13], elements[14], elements[15])
    val row0 inline get() = Vec4(elements[0], elements[4], elements[8], elements[12])
    val row1 inline get() = Vec4(elements[1], elements[5], elements[9], elements[13])
    val row2 inline get() = Vec4(elements[2], elements[6], elements[10], elements[14])
    val row3 inline get() = Vec4(elements[3], elements[7], elements[11], elements[15])

    constructor(){ elements = FloatArray(4*4){0f} }

    constructor(diagonal: Float) {
        elements = floatArrayOf(diagonal, 0f, 0f, 0f,
            0f, diagonal, 0f, 0f,
            0f, 0f, diagonal, 0f,
            0f, 0f, 0f, diagonal)
    }

    // NOTE: Column-major ordering
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
        elements = floatArrayOf(mat3[0], mat3[1], mat3[2], 0f,
            mat3[3], mat3[4], mat3[5], 0f,
            mat3[6], mat3[7], mat3[8], 0f,
            0f,      0f,      0f, 1f)
    }

    operator fun get(i: Int) = elements[i]

    operator fun times(other: Mat4) = Mat4(
        (this[0] * other[0]) + (this[4] * other[1]) + (this[8] * other[2]) + (this[12] * other[3]),
        (this[1] * other[0]) + (this[5] * other[1]) + (this[9] * other[2]) + (this[13] * other[3]),
        (this[2] * other[0]) + (this[6] * other[1]) + (this[10] * other[2]) + (this[14] * other[3]),
        (this[3] * other[0]) + (this[7] * other[1]) + (this[11] * other[2]) + (this[15] * other[3]),
        (this[0] * other[4]) + (this[4] * other[5]) + (this[8] * other[6]) + (this[12] * other[7]),
        (this[1] * other[4]) + (this[5] * other[5]) + (this[9] * other[6]) + (this[13] * other[7]),
        (this[2] * other[4]) + (this[6] * other[5]) + (this[10] * other[6]) + (this[14] * other[7]),
        (this[3] * other[4]) + (this[7] * other[5]) + (this[11] * other[6]) + (this[15] * other[7]),
        (this[0] * other[8]) + (this[4] * other[9]) + (this[8] * other[10]) + (this[12] * other[11]),
        (this[1] * other[8]) + (this[5] * other[9]) + (this[9] * other[10]) + (this[13] * other[11]),
        (this[2] * other[8]) + (this[6] * other[9]) + (this[10] * other[10]) + (this[14] * other[11]),
        (this[3] * other[8]) + (this[7] * other[9]) + (this[11] * other[10]) + (this[15] * other[11]),
        (this[0] * other[12]) + (this[4] * other[13]) + (this[8] * other[14]) + (this[12] * other[15]),
        (this[1] * other[12]) + (this[5] * other[13]) + (this[9] * other[14]) + (this[13] * other[15]),
        (this[2] * other[12]) + (this[6] * other[13]) + (this[10] * other[14]) + (this[14] * other[15]),
        (this[3] * other[12]) + (this[7] * other[13]) + (this[11] * other[14]) + (this[15] * other[15])
    )

    operator fun times(other: Mat3) = Mat4(
        (this[0] * other[0]) + (this[4] * other[1]) + (this[8] * other[2]),
        (this[1] * other[0]) + (this[5] * other[1]) + (this[9] * other[2]),
        (this[2] * other[0]) + (this[6] * other[1]) + (this[10] * other[2]),
        (this[3] * other[0]) + (this[7] * other[1]) + (this[11] * other[2]),
        (this[0] * other[3]) + (this[4] * other[4]) + (this[8] * other[5]),
        (this[1] * other[3]) + (this[5] * other[4]) + (this[9] * other[5]),
        (this[2] * other[3]) + (this[6] * other[4]) + (this[10] * other[5]),
        (this[3] * other[3]) + (this[7] * other[4]) + (this[11] * other[5]),
        (this[0] * other[6]) + (this[4] * other[7]) + (this[8] * other[8]),
        (this[1] * other[6]) + (this[5] * other[7]) + (this[9] * other[8]),
        (this[2] * other[6]) + (this[6] * other[7]) + (this[10] * other[8]),
        (this[3] * other[6]) + (this[7] * other[7]) + (this[11] * other[8]),
        this[12],
        this[13],
        this[14],
        this[15]
    )

    // Vec4 as column vector
    operator fun times(v: Vec4) = Vec4( // Vec4 treated as column vector
        this[0]*v[0] + this[4]*v[1] + this[8]*v[2] + this[12]*v[3],
        this[1]*v[0] + this[5]*v[1] + this[9]*v[2] + this[13]*v[3],
        this[2]*v[0] + this[6]*v[1] + this[10]*v[2] + this[14]*v[3],
        this[3]*v[0] + this[7]*v[1] + this[11]*v[2] + this[15]*v[3]
    )

    companion object {
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