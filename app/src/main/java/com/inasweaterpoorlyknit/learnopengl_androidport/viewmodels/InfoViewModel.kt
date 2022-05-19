package com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import com.inasweaterpoorlyknit.learnopengl_androidport.OpenGLScenesApplication
import com.inasweaterpoorlyknit.learnopengl_androidport.getMengerSpongeResolutionIndex
import com.inasweaterpoorlyknit.learnopengl_androidport.getSharedPreferences
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MengerPrisonScene
import com.inasweaterpoorlyknit.learnopengl_androidport.setMengerSpongeResolutionIndex

class InfoViewModel(application: Application) : AndroidViewModel(application) {

    private val scenesApp = application as OpenGLScenesApplication
    private val sharedPreferences = application.getSharedPreferences()

    companion object {
        val mengerPrisonResolutions: Array<String> = Array(MengerPrisonScene.resolutionFactorOptions.size) {
            "${(MengerPrisonScene.resolutionFactorOptions[it] * 100.0f).toInt()}%"
        }
    }

    private val _webRequest = MutableLiveData<String>()
    val webRequest: LiveData<String>
        get() = _webRequest

    fun getDarkMode(): Boolean = (scenesApp.darkMode.value ?: true)
    fun getMengerSpongeResolutionIndex(): Int = sharedPreferences.getMengerSpongeResolutionIndex()

    fun onContactPress() { _webRequest.value = "https://lucodivo.github.io/about.html" }
    fun onSourcePress() { _webRequest.value = "https://github.com/Lucodivo/OpenGLScenes_Android" }
    fun onNightModeToggle(newState: Boolean) = scenesApp.setDarkMode(newState)
    fun onMengerPrisonResolutionSelected(index: Int) = sharedPreferences.setMengerSpongeResolutionIndex(index)
}