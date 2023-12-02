package com.inasweaterpoorlyknit.learnopengl_androidport.graphics

import android.content.Context
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.util.Log
import android.widget.Toast
import com.inasweaterpoorlyknit.Mat3
import com.inasweaterpoorlyknit.learnopengl_androidport.R

/*
 TODO: This helper never acknowledges that we are translating from Android supplied column-major matrices
    into custom row-major matrices. Potential bugs/optimizations to be fixed/had.
*/
class RotationSensorHelper: SensorEventListener {

    private var firstSensorVals_rotationVector: FloatArray? = null
    private var lastSensorValues_rotationVector: FloatArray? = null
    private val scratchMat = FloatArray(3 * 3) // don't expect anything to be in here unless you just wrote to it

    fun init(context: Context) {
        val sensorManager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
        val sensor: Sensor? = sensorManager.getDefaultSensor(Sensor.TYPE_GAME_ROTATION_VECTOR)

        // enable rotation sensor if available
        if(sensor != null) {
            sensorManager.registerListener(this, sensor, SensorManager.SENSOR_DELAY_GAME)
        } else {
            Toast.makeText(context, R.string.no_rotation_sensor, Toast.LENGTH_LONG).show()
        }
    }

    fun reset() {
        firstSensorVals_rotationVector = null
        lastSensorValues_rotationVector = null
    }

    fun deinit(context: Context) {
        // Turn our sensor off on detached
        val sensorManager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
        sensorManager.unregisterListener(this)
        firstSensorVals_rotationVector = null
        lastSensorValues_rotationVector = null
    }

    fun getRotationMatrix(orientation: Orientation): Mat3 {
        if(lastSensorValues_rotationVector == null) { return Mat3(1.0f) }
        if(firstSensorVals_rotationVector == null) {
            // It seems like the rotation sensor can return the identity matrix. Check for that. If identity, don't further initialize.
            // Rotation values:
            //      values[0]: x*sin(θ/2)
            //      values[1]: y*sin(θ/2)
            //      values[2]: z*sin(θ/2)
            //      values[3]: cos(θ/2)
            val values = lastSensorValues_rotationVector!!
            if(values[3] == 1.0f) {
                Log.e("Rotation Sensor Error", "Rotation sensor provided an identity matrix")
                return Mat3(1.0f)
            }
            firstSensorVals_rotationVector = values.copyOf()
        }

        // TODO: Consider not converting the first sensor to a rotation matrix every frame
        val lastSensor_rotationMat = FloatArray(3 * 3)
        SensorManager.getRotationMatrixFromVector(lastSensor_rotationMat, lastSensorValues_rotationVector)
        val firstSensor_rotationMat = FloatArray(3 * 3)
        SensorManager.getRotationMatrixFromVector(firstSensor_rotationMat, firstSensorVals_rotationVector)

        when(orientation) {
            Orientation.Portrait -> {
                /* nada */
            }
            Orientation.Landscape -> {
                SensorManager.remapCoordinateSystem(lastSensor_rotationMat, SensorManager.AXIS_Y, SensorManager.AXIS_MINUS_X, scratchMat)
                scratchMat.copyInto(lastSensor_rotationMat)
                SensorManager.remapCoordinateSystem(firstSensor_rotationMat, SensorManager.AXIS_Y, SensorManager.AXIS_MINUS_X, scratchMat)
                scratchMat.copyInto(firstSensor_rotationMat)
            }
            Orientation.PortraitReverse -> {
                SensorManager.remapCoordinateSystem(lastSensor_rotationMat, SensorManager.AXIS_MINUS_X, SensorManager.AXIS_MINUS_Y, scratchMat)
                scratchMat.copyInto(lastSensor_rotationMat)
                SensorManager.remapCoordinateSystem(firstSensor_rotationMat, SensorManager.AXIS_MINUS_X, SensorManager.AXIS_MINUS_Y, scratchMat)
                scratchMat.copyInto(firstSensor_rotationMat)
            }
            Orientation.LandscapeReverse -> {
                SensorManager.remapCoordinateSystem(lastSensor_rotationMat, SensorManager.AXIS_MINUS_Y, SensorManager.AXIS_X, scratchMat)
                scratchMat.copyInto(lastSensor_rotationMat)
                SensorManager.remapCoordinateSystem(firstSensor_rotationMat, SensorManager.AXIS_MINUS_Y, SensorManager.AXIS_X, scratchMat)
                scratchMat.copyInto(firstSensor_rotationMat)
            }
        }

        // Problem: Rotation values are right-handed coordinate system with Z pointing out of the screen
        // Problem: This causes rotation around the Z-axis to occur in the opposite direction
        // Solution: Flip rotation along the X-axis and Y-axis by negating the Z values in the rotation matrix
        // Solution: Our desired rotation matrix is now the inverse of the matrix we have
        val lastRotationMat_FlippedZ_Inverse = Mat3(lastSensor_rotationMat[0], lastSensor_rotationMat[3], lastSensor_rotationMat[6], // transpose is inverse for pure rotation matrix
                                            lastSensor_rotationMat[1], lastSensor_rotationMat[4], lastSensor_rotationMat[7],
                                            -lastSensor_rotationMat[2], -lastSensor_rotationMat[5], -lastSensor_rotationMat[8])
        val firstRotationMat_ZNegated = Mat3(firstSensor_rotationMat[0], firstSensor_rotationMat[1], -firstSensor_rotationMat[2],
                                            firstSensor_rotationMat[3], firstSensor_rotationMat[4], -firstSensor_rotationMat[5],
                                            firstSensor_rotationMat[6], firstSensor_rotationMat[7], -firstSensor_rotationMat[8])
        return (firstRotationMat_ZNegated * lastRotationMat_FlippedZ_Inverse) // transpose is inverse for pure rotation matrix

        // NOTE: Below is a slightly effiecient but more straight forward solution
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
                lastSensorValues_rotationVector = event.values
            }
        }
    }

    override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) {}
}