package com.inasweaterpoorlyknit.learnopengl_androidport

import android.content.Context
import android.content.SharedPreferences
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MandelbrotScene

import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MengerPrisonScene

object SharedPrefKeys {
    const val darkModeName = "dark_mode"
    const val cachedSystemDarkMode = "dark_mode_system"
    const val mengerPrisonResolutionIndex = "menger_prison_resolution_index"
    const val mandelbrotScene = "mandelbrot_color_index"
}

fun Context.getSharedPreferences(): SharedPreferences {
    val globalSharedPreferencesName = "global_preferences"
    return getSharedPreferences(globalSharedPreferencesName, 0)
}

fun SharedPreferences.darkModeInitialized() = contains(SharedPrefKeys.darkModeName)

fun SharedPreferences.getDarkMode(default: Boolean = true) = getBoolean(SharedPrefKeys.darkModeName, default)
fun SharedPreferences.setDarkMode(darkMode: Boolean) = edit().putBoolean(SharedPrefKeys.darkModeName, darkMode).apply()

fun SharedPreferences.setCachedSystemDarkMode(systemDarkMode: Boolean = true) = edit().putBoolean(SharedPrefKeys.cachedSystemDarkMode, systemDarkMode).apply()
fun SharedPreferences.getCachedSystemDarkMode(default: Boolean = true) = getBoolean(SharedPrefKeys.cachedSystemDarkMode, default)

fun SharedPreferences.getMengerSpongeResolutionIndex(default: Int = MengerPrisonScene.defaultResolutionIndex) = getInt(SharedPrefKeys.mengerPrisonResolutionIndex, default)
fun SharedPreferences.setMengerSpongeResolutionIndex(resIndex: Int) = edit().putInt(SharedPrefKeys.mengerPrisonResolutionIndex, resIndex).apply()

fun SharedPreferences.getMandelbrotColorIndex(default: Int = MandelbrotScene.defaultColorIndex) = getInt(SharedPrefKeys.mandelbrotScene, default)
fun SharedPreferences.setMandelbrotColorIndex(colorIndex: Int) = edit().putInt(SharedPrefKeys.mandelbrotScene, colorIndex).apply()