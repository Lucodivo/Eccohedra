package com.inasweaterpoorlyknit

import kotlin.test.Test
import kotlin.test.assertEquals

class MathUnitTests {
    @Test
    fun `Vec2 - lenSq, len, normalize`() {
        // 25 = 9 + 16
        val v = Vec2(3f, 4f)
        val expectedLength = 5f

        val expectedNormalized = Vec2(v.x/expectedLength, v.y/expectedLength)

        assertEquals(expectedNormalized, v.normalized)
    }

    @Test
    fun `Vec2 - dot product`() {
        val v = Vec2(1f, 2f)

        val xScaled = Vec2(5f, 0f)
        val yScaled = Vec2(0f, 6f)

        val expected_XScaled = v.x * xScaled.x
        val expected_YScaled = v.y * yScaled.y

        val actual_XScaled = v.dot(xScaled)
        val actual_YScaled = v.dot(yScaled)

        assertEquals(expected_XScaled, actual_XScaled)
        assertEquals(expected_YScaled, actual_YScaled)
    }

    @Test
    fun `Vec3 - lenSq, len, normalize`() {
        // 81 = 64 + 16 + 1
        val v = Vec3(8f, 4f, 1f)
        val expectedLength = 9f

        val expectedNormalized = Vec3(v.x/expectedLength, v.y/expectedLength, v.z/expectedLength)

        assertEquals(expectedNormalized, v.normalized)
    }

    @Test
    fun `Vec3 - dot product`() {
        val v = Vec3(1f, 2f, 3f)

        val xScaled = Vec3(5f, 0f, 0f)
        val yScaled = Vec3(0f, 6f, 0f)
        val zScaled = Vec3(0f, 0f, 7f)

        val expected_XScaled = v.x * xScaled.x
        val expected_YScaled = v.y * yScaled.y
        val expected_ZScaled = v.z * zScaled.z

        val actual_XScaled = v.dot(xScaled)
        val actual_YScaled = v.dot(yScaled)
        val actual_ZScaled = v.dot(zScaled)

        assertEquals(expected_XScaled, actual_XScaled)
        assertEquals(expected_YScaled, actual_YScaled)
        assertEquals(expected_ZScaled, actual_ZScaled)
    }

    @Test
    fun `Vec3 - cross product`() {
        val x = Vec3(1f, 0f, 0f)
        val y = Vec3(0f, 1f, 0f)
        val z = Vec3(0f, 0f, 1f)
        val negX = Vec3(-1f, 0f, 0f)
        val negY = Vec3(0f, -1f, 0f)
        val negZ = Vec3(0f, 0f, -1f)

        assertEquals(y.cross(z), x)
        assertEquals(z.cross(y), negX)
        assertEquals(z.cross(x), y)
        assertEquals(x.cross(z), negY)
        assertEquals(x.cross(y), z)
        assertEquals(y.cross(x), negZ)
    }

    @Test
    fun `Vec4 - lenSq, len, normalize`() {
        // Found using list of Lagrange's Four Square Theorem
        // 81 = 36 + 25 + 16 + 4
        val v = Vec4(6f, 5f, 4f, 2f)
        val expectedLength = 9f

        val expectedNormalized = Vec4(v.x/expectedLength, v.y/expectedLength, v.z/expectedLength, v.w/expectedLength)

        assertEquals(expectedNormalized, v.normalized)
    }

    @Test
    fun `Vec4 - dot product`() {
        val v = Vec4(1f, 2f, 3f, 4f)

        val xScaled = Vec4(5f, 0f, 0f, 0f)
        val yScaled = Vec4(0f, 6f, 0f, 0f)
        val zScaled = Vec4(0f, 0f, 7f, 0f)
        val wScaled = Vec4(0f, 0f, 0f, 8f)

        val expected_XScaled = v.x * xScaled.x
        val expected_YScaled = v.y * yScaled.y
        val expected_ZScaled = v.z * zScaled.z
        val expected_WScaled = v.w * wScaled.w

        val actual_XScaled = v.dot(xScaled)
        val actual_YScaled = v.dot(yScaled)
        val actual_ZScaled = v.dot(zScaled)
        val actual_WScaled = v.dot(wScaled)

        assertEquals(expected_XScaled, actual_XScaled)
        assertEquals(expected_YScaled, actual_YScaled)
        assertEquals(expected_ZScaled, actual_ZScaled)
        assertEquals(expected_WScaled, actual_WScaled)
    }

    @Test
    fun `Mat2 - multiply by diagonal scale matrix`() {

        val scaleByFour = Mat2(4f)
        val originalMat = Mat2(
            0f, 0.25f,
            0.5f, 1f
        )
        val expectedResult = Mat2(
            0f, 1f,
            2f, 4f,
        )

        val actualResult1 = originalMat * scaleByFour
        val actualResult2 = scaleByFour * originalMat

        assertEquals(expectedResult, actualResult1)
        assertEquals(expectedResult, actualResult2)
    }

    @Test
    fun `Mat2 - multiply by scaling and rotating matrix`() {
        val xScale = 2f
        val yScale = 4f
        val rotationScaleMat = Mat2(
            0f, xScale,
            yScale, 0f
        )

        val point = Vec2(130f, 1.1f)
        val expectedResult = Vec2(point.y * xScale, point.x * yScale)
        val expectedResultDotCalc = Vec2(
            point.dot(rotationScaleMat.row0),
            point.dot(rotationScaleMat.row1)
        )

        val actualResult = rotationScaleMat * point

        assertEquals(expectedResult, actualResult)
        assertEquals(expectedResultDotCalc, actualResult)
    }

    @Test
    fun `Mat2 - multiply to transform point to new coordinate space`() {
        val xScale = 2f
        val yScale = 4f
        val transformationMat = Mat2(
            0f, xScale,
            yScale, 0f
        )

        val point = Vec2(130f, 1.1f)
        val expectedResult = Vec2(
            point.dot(transformationMat.col0),
            point.dot(transformationMat.col1)
        )

        val actualResult = point * transformationMat

        assertEquals(expectedResult, actualResult)
    }

    @Test
    fun `Mat3 - multiply by diagonal scale matrix`() {

        val scaleByFour = Mat3(4f)
        val originalMat = Mat3(
            0f, 0f, 0f,
            0.25f, 0.25f, 0.25f,
            0.5f, 0.5f, 0.5f
        )
        val expectedResult = Mat3(
            0f, 0f, 0f,
            1f, 1f, 1f,
            2f, 2f, 2f,
        )

        val actualResult1 = originalMat * scaleByFour
        val actualResult2 = scaleByFour * originalMat

        assertEquals(expectedResult, actualResult1)
        assertEquals(expectedResult, actualResult2)
    }

    @Test
    fun `Mat3 - multiply by scaling and rotating matrix`() {

        /*
         rotate 120 degrees around (1,1,1) and scale where
         +X goes to +2Y, +Y goes to +4Z, +Z goes to +6X
        */
        val xScale = 2f
        val yScale = 4f
        val zScale = 6f
        val rotationScaleMat = Mat3(
            0f, xScale, 0f,
            0f, 0f, yScale,
            zScale, 0f, 0f
        )

        val point = Vec3(130f, 1.1f, 1731.73f)
        val expectedResult = Vec3(point.y * xScale, point.z * yScale, point.x * zScale)
        val expectedResultDotCalc = Vec3(
            point.dot(rotationScaleMat.row0),
            point.dot(rotationScaleMat.row1),
            point.dot(rotationScaleMat.row2)
        )

        val actualResult = rotationScaleMat * point

        assertEquals(expectedResult, actualResult)
        assertEquals(expectedResultDotCalc, actualResult)
    }

    @Test
    fun `Mat3 - multiply to transform point to new coordinate space`() {

        /*
         new space is rotated 120 degrees around (1,1,1) and scaled where
         +X goes to +2Y, +Y goes to +4Z, +Z goes to +6X
        */
        val xScale = 2f
        val yScale = 4f
        val zScale = 6f
        val transformationMat = Mat3(
            0f, xScale, 0f,
            0f, 0f, yScale,
            zScale, 0f, 0f
        )

        val point = Vec3(130f, 1.1f, 1731.73f)
        val expectedResult = Vec3(
            point.dot(transformationMat.col0),
            point.dot(transformationMat.col1),
            point.dot(transformationMat.col2)
        )

        val actualResult = point * transformationMat

        assertEquals(expectedResult, actualResult)
    }

    @Test
    fun `Mat4 - multiply by diagonal scale matrix`() {

        val scaleByFour = Mat4(4f)
        val originalMat = Mat4(
            0f, 0f, 0f, 0f,
            0.25f, 0.25f, 0.25f, 0.25f,
            0.5f, 0.5f, 0.5f, 0.5f,
            1f, 1f, 1f, 1f
        )
        val expectedResult = Mat4(
            0f, 0f, 0f, 0f,
            1f, 1f, 1f, 1f,
            2f, 2f, 2f, 2f,
            4f, 4f, 4f, 4f
        )

        val actualResult1 = originalMat * scaleByFour
        val actualResult2 = scaleByFour * originalMat

        assertEquals(expectedResult, actualResult1)
        assertEquals(expectedResult, actualResult2)
    }

    @Test
    fun `Mat4 - multiply by scaling and rotating matrix`() {

        /*
         rotate 120 degrees around (1,1,1) and scale where
         +X goes to +2Y, +Y goes to +4Z, +Z goes to +6X
        */
        val xScale = 2f
        val yScale = 4f
        val zScale = 6f
        val wScale = 8f
        val rotationScaleMat = Mat4(
            0f, xScale, 0f, 0f,
            0f, 0f, yScale, 0f,
            zScale, 0f, 0f, 0f,
            0f, 0f, 0f, wScale
        )

        val point = Vec4(130f, 1.1f, 1731.73f, 1f)
        val expectedResult = Vec4(point.y * xScale, point.z * yScale, point.x * zScale, point.w * wScale)
        val expectedResultDotCalc = Vec4(
            point.dot(rotationScaleMat.row0),
            point.dot(rotationScaleMat.row1),
            point.dot(rotationScaleMat.row2),
            point.dot(rotationScaleMat.row3),
        )

        val actualResult = rotationScaleMat * point

        assertEquals(expectedResult, actualResult)
        assertEquals(expectedResultDotCalc, actualResult)
    }

    @Test
    fun `Mat4 - multiply to transform point to new coordinate space`() {

        /*
         new space is rotated 120 degrees around (1,1,1) and scaled where
         +X goes to +2Y, +Y goes to +4Z, +Z goes to +6X
        */
        val xScale = 2f
        val yScale = 4f
        val zScale = 6f
        val wScale = 8f
        val transformationMat = Mat4(
            0f, xScale, 0f, 0f,
            0f, 0f, yScale, 0f,
            zScale, 0f, 0f, 0f,
            0f, 0f, 0f, wScale
        )

        val point = Vec4(130f, 1.1f, 1731.73f, 1f)
        val expectedResult = Vec4(
            point.dot(transformationMat.col0),
            point.dot(transformationMat.col1),
            point.dot(transformationMat.col2),
            point.dot(transformationMat.col3),
        )

        val actualResult = point * transformationMat

        assertEquals(expectedResult, actualResult)
    }
}
