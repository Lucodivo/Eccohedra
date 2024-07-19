package com.inasweaterpoorlyknit.scenes.graphics

import android.content.Context
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.util.Log
import android.widget.Toast
import com.inasweaterpoorlyknit.noopmath.Mat3
import com.inasweaterpoorlyknit.scenes.R
import com.inasweaterpoorlyknit.scenes.graphics.RotationSensorHelper.ScratchMats.firstSensorRotationMat
import com.inasweaterpoorlyknit.scenes.graphics.RotationSensorHelper.ScratchMats.lastSensorRotationMat
import com.inasweaterpoorlyknit.scenes.graphics.RotationSensorHelper.ScratchMats.scratchMat

class RotationSensorHelper(context: Context): SensorEventListener {

    private var firstSensorRotationVector: FloatArray? = null
    private var lastSensorRotationVector: FloatArray? = null

    // don't expect anything to be in these unless you just wrote to it
    private object ScratchMats {
        val lastSensorRotationMat = FloatArray(3 * 3)
        val firstSensorRotationMat = FloatArray(3 * 3)
        val scratchMat = FloatArray(3 * 3)
    }

    private val sensorManager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
    private val sensor: Sensor? = sensorManager.getDefaultSensor(Sensor.TYPE_GAME_ROTATION_VECTOR)
    private val noRotationToast = Toast.makeText(context, R.string.no_rotation_sensor, Toast.LENGTH_LONG)

    fun init() {
        // enable rotation sensor if available
        reset()
        if(sensor != null) sensorManager.registerListener(this, sensor, SensorManager.SENSOR_DELAY_GAME) else noRotationToast.show()
    }

    fun reset() {
        firstSensorRotationVector = null
        lastSensorRotationVector = null
    }

    fun deinit() {
        // Turn our sensor off on detached
        sensorManager.unregisterListener(this)
        firstSensorRotationVector = null
        lastSensorRotationVector = null
    }

    fun getRotationMatrix(orientation: Orientation): Mat3 {
        if(lastSensorRotationVector == null) { return Mat3(1.0f) }
        if(firstSensorRotationVector == null) {
            // It seems like the rotation sensor can return the identity matrix. Check for that. If identity, don't further initialize.
            // Rotation values:
            //      values[0]: x*sin(θ/2)
            //      values[1]: y*sin(θ/2)
            //      values[2]: z*sin(θ/2)
            //      values[3]: cos(θ/2)
            val values = lastSensorRotationVector!!
            if(values[3] == 1.0f) {
                Log.e("Rotation Sensor Error", "Rotation sensor provided an identity matrix")
                return Mat3(1.0f)
            }
            firstSensorRotationVector = values.copyOf()
        }

        // TODO: Consider not converting the first sensor to a rotation matrix every frame
        SensorManager.getRotationMatrixFromVector(lastSensorRotationMat, lastSensorRotationVector)
        SensorManager.getRotationMatrixFromVector(firstSensorRotationMat, firstSensorRotationVector)

        when(orientation) {
            Orientation.Portrait -> { /* rotation matrix is already synced to orientation */ }
            Orientation.Landscape -> {
                SensorManager.remapCoordinateSystem(lastSensorRotationMat, SensorManager.AXIS_Y, SensorManager.AXIS_MINUS_X, scratchMat)
                scratchMat.copyInto(lastSensorRotationMat)
                SensorManager.remapCoordinateSystem(firstSensorRotationMat, SensorManager.AXIS_Y, SensorManager.AXIS_MINUS_X, scratchMat)
                scratchMat.copyInto(firstSensorRotationMat)
            }
            Orientation.PortraitReverse -> {
                SensorManager.remapCoordinateSystem(lastSensorRotationMat, SensorManager.AXIS_MINUS_X, SensorManager.AXIS_MINUS_Y, scratchMat)
                scratchMat.copyInto(lastSensorRotationMat)
                SensorManager.remapCoordinateSystem(firstSensorRotationMat, SensorManager.AXIS_MINUS_X, SensorManager.AXIS_MINUS_Y, scratchMat)
                scratchMat.copyInto(firstSensorRotationMat)
            }
            Orientation.LandscapeReverse -> {
                SensorManager.remapCoordinateSystem(lastSensorRotationMat, SensorManager.AXIS_MINUS_Y, SensorManager.AXIS_X, scratchMat)
                scratchMat.copyInto(lastSensorRotationMat)
                SensorManager.remapCoordinateSystem(firstSensorRotationMat, SensorManager.AXIS_MINUS_Y, SensorManager.AXIS_X, scratchMat)
                scratchMat.copyInto(firstSensorRotationMat)
            }
        }

        // Problem: Rotation values are right-handed coordinate system with Z pointing out of the screen
        // Problem: This causes rotation around the Z-axis to occur in the opposite direction
        // Solution: Flip rotation along the X-axis and Y-axis by negating the Z values in the rotation matrix
        // Solution: Our desired rotation matrix is now the inverse of the matrix we have
        val lastRotationMat_FlippedZ_Inverse = Mat3(lastSensorRotationMat[0], lastSensorRotationMat[3], lastSensorRotationMat[6], // transpose is inverse for pure rotation matrix
                                            lastSensorRotationMat[1], lastSensorRotationMat[4], lastSensorRotationMat[7],
                                            -lastSensorRotationMat[2], -lastSensorRotationMat[5], -lastSensorRotationMat[8])
        val firstRotationMat_ZNegated = Mat3(firstSensorRotationMat[0], firstSensorRotationMat[1], -firstSensorRotationMat[2],
                                            firstSensorRotationMat[3], firstSensorRotationMat[4], -firstSensorRotationMat[5],
                                            firstSensorRotationMat[6], firstSensorRotationMat[7], -firstSensorRotationMat[8])
        return (firstRotationMat_ZNegated * lastRotationMat_FlippedZ_Inverse) // transpose is inverse for pure rotation matrix

        // NOTE: Below is a slightly less efficient but more straight forward solution
//        val lastRotationMat_FlippedZ = Mat3(lastSensor_rotationMat[0], lastSensor_rotationMat[1], -lastSensor_rotationMat[2],
//                                            lastSensor_rotationMat[3], lastSensor_rotationMat[4], -lastSensor_rotationMat[5],
//                                            lastSensor_rotationMat[6], lastSensor_rotationMat[7], -lastSensor_rotationMat[8])
//        val firstRotationMat_FlippedZ_Inverse = Mat3(firstSensor_rotationMat[0], firstSensor_rotationMat[3], firstSensor_rotationMat[6], // transpose is inverse for pure rotation matrix
//                                                    firstSensor_rotationMat[1], firstSensor_rotationMat[4], firstSensor_rotationMat[7],
//                                                    -firstSensor_rotationMat[2], -firstSensor_rotationMat[5], -firstSensor_rotationMat[8])
//        return (lastRotationMat_FlippedZ * firstRotationMat_FlippedZ_Inverse).transpose() // transpose is inverse for pure rotation matrix
    }

    override fun onSensorChanged(event: SensorEvent) {
        when(event.sensor.type){
            Sensor.TYPE_GAME_ROTATION_VECTOR -> {
                lastSensorRotationVector = event.values
            }
        }
    }

    override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) {}
}