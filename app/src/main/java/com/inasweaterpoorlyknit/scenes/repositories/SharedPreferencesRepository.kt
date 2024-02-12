package com.inasweaterpoorlyknit.scenes.repositories

import com.inasweaterpoorlyknit.scenes.graphics.scenes.MengerPrisonScene
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MandelbrotScene

import android.content.Context
import android.content.Context.MODE_PRIVATE

class SharedPreferencesRepository(context: Context) {
    // name of shared preferences file
    private val globalSharedPreferencesName = "global_preferences"
    // MODE_PRIVATE means that the file can only be accessed by the calling application
    private val globalSharedPreferences = context.getSharedPreferences(globalSharedPreferencesName, MODE_PRIVATE)

    private object SharedPrefKeys {
        const val DARK_MODE_NAME = "dark_mode"
        const val CACHED_SYSTEM_DARK_MODE = "dark_mode_system"
        const val MENGER_PRISON_RESOLUTION_INDEX = "menger_prison_resolution_index"
        const val MANDELBROT_SCENE = "mandelbrot_color_index"
    }

    fun getMengerSpongeResolutionIndex(default: Int = MengerPrisonScene.DEFAULT_RESOLUTION_INDEX) = globalSharedPreferences.getInt(SharedPrefKeys.MENGER_PRISON_RESOLUTION_INDEX, default)
    fun setMengerSpongeResolutionIndex(resIndex: Int) = globalSharedPreferences.edit().putInt(SharedPrefKeys.MENGER_PRISON_RESOLUTION_INDEX, resIndex).apply()

    fun getMandelbrotColorIndex(default: Int = MandelbrotScene.DEFAULT_COLOR_INDEX) = globalSharedPreferences.getInt(SharedPrefKeys.MANDELBROT_SCENE, default)
    fun setMandelbrotColorIndex(colorIndex: Int) = globalSharedPreferences.edit().putInt(SharedPrefKeys.MANDELBROT_SCENE, colorIndex).apply()
}