package com.inasweaterpoorlyknit.learnopengl_androidport

import android.content.SharedPreferences

private const val darkModeName = "dark_mode"
private const val cachedSystemDarkMode = "dark_mode_system"

fun SharedPreferences.darkModeInitialized() = contains(darkModeName)

fun SharedPreferences.getDarkMode(default: Boolean = true) = getBoolean(darkModeName, default)
fun SharedPreferences.setDarkMode(darkMode: Boolean) = edit().putBoolean(darkModeName, darkMode).apply()

fun SharedPreferences.setCachedSystemDarkMode(systemDarkMode: Boolean = true) = edit().putBoolean(cachedSystemDarkMode, systemDarkMode).apply()
fun SharedPreferences.getCachedSystemDarkMode(default: Boolean = true) = getBoolean(cachedSystemDarkMode, default)