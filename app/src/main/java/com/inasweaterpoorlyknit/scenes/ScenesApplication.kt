package com.inasweaterpoorlyknit.scenes

import android.app.Application
import android.content.SharedPreferences
import android.content.res.Configuration
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import com.google.android.material.color.DynamicColors

// TODO: Hilt
//@HiltAndroidApp
class ScenesApplication : Application() {

    private val _darkMode = MutableLiveData<Boolean>()
    val darkMode: LiveData<Boolean>
        get() = _darkMode

    private lateinit var sharedPreferences: SharedPreferences

    override fun onCreate() {
        super.onCreate()
        DynamicColors.applyToActivitiesIfAvailable(this)

        sharedPreferences = getSharedPreferences()

        // configure dark mode
        val systemDarkMode = getSystemDarkMode(resources.configuration)
        if(sharedPreferences.darkModeInitialized()) {
            if(systemDarkMode != sharedPreferences.getCachedSystemDarkMode()) { // but user has changed system wide dark mode
                setDarkMode(systemDarkMode)
                sharedPreferences.setCachedSystemDarkMode(systemDarkMode)
            } else {
                _darkMode.value = sharedPreferences.getDarkMode()
            }
        } else { // default to system dark mode
            setDarkMode(systemDarkMode)
            sharedPreferences.setCachedSystemDarkMode(systemDarkMode)
        }
    }

    fun setDarkMode(darkMode: Boolean) {
        sharedPreferences.setDarkMode(darkMode)
        _darkMode.value = darkMode
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        val darkMode = getSystemDarkMode(newConfig)
        if(darkMode != _darkMode.value) {
            setDarkMode(darkMode)
            sharedPreferences.setCachedSystemDarkMode(darkMode)
        }
    }

    private fun getSystemDarkMode(configuration: Configuration): Boolean {
        return (configuration.uiMode and Configuration.UI_MODE_NIGHT_MASK) == Configuration.UI_MODE_NIGHT_YES
    }
}