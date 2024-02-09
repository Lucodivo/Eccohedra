package com.inasweaterpoorlyknit.scenes.viewmodels

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.MutableLiveData
import com.inasweaterpoorlyknit.scenes.ScenesApplication
import com.inasweaterpoorlyknit.scenes.getMandelbrotColorIndex
import com.inasweaterpoorlyknit.scenes.getMengerSpongeResolutionIndex
import com.inasweaterpoorlyknit.scenes.getSharedPreferences
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MandelbrotScene
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MengerPrisonScene
import com.inasweaterpoorlyknit.scenes.setMandelbrotColorIndex
import com.inasweaterpoorlyknit.scenes.setMengerSpongeResolutionIndex

class SettingsViewModel(application: Application) : AndroidViewModel(application) {

    private val scenesApp = application as ScenesApplication
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