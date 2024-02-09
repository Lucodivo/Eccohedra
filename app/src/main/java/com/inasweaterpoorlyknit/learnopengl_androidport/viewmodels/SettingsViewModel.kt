package com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import com.inasweaterpoorlyknit.learnopengl_androidport.OpenGLScenesApplication
import com.inasweaterpoorlyknit.learnopengl_androidport.getMandelbrotColorIndex
import com.inasweaterpoorlyknit.learnopengl_androidport.getMengerSpongeResolutionIndex
import com.inasweaterpoorlyknit.learnopengl_androidport.getSharedPreferences
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MandelbrotScene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MengerPrisonScene
import com.inasweaterpoorlyknit.learnopengl_androidport.setMandelbrotColorIndex
import com.inasweaterpoorlyknit.learnopengl_androidport.setMengerSpongeResolutionIndex

class SettingsViewModel(application: Application) : AndroidViewModel(application) {

    private val scenesApp = application as OpenGLScenesApplication
    private val sharedPreferences = application.getSharedPreferences()

    companion object {
        val mengerPrisonResolutions: Array<String> = Array(MengerPrisonScene.resolutionFactorOptions.size) {
            "${(MengerPrisonScene.resolutionFactorOptions[it] * 100.0f).toInt()}%"
        }
        val mandelbrotColors: Array<String> = Array(MandelbrotScene.colors.size) { MandelbrotScene.colors[it].name }
        val websiteUrl = "https://lucodivo.github.io"
        val sourceUrl = "https://github.com/Lucodivo/ScenesMobile"
    }

    private val _webRequest = MutableLiveData<String>()

    fun getMengerSpongeResolutionIndex(): Int = sharedPreferences.getMengerSpongeResolutionIndex()
    fun getMandelbrotColorIndex(): Int = sharedPreferences.getMandelbrotColorIndex()

    fun onNightModeToggle(newState: Boolean) = scenesApp.setDarkMode(newState)
    fun onMengerPrisonResolutionSelected(index: Int) = sharedPreferences.setMengerSpongeResolutionIndex(index)
    fun onMandelbrotColorSelected(index: Int) = sharedPreferences.setMandelbrotColorIndex(index)
}