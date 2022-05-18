package com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import com.inasweaterpoorlyknit.learnopengl_androidport.OpenGLScenesApplication

class InfoViewModel(application: Application) : AndroidViewModel(application) {

    private val scenesApp = application as OpenGLScenesApplication

    private val _webRequest = MutableLiveData<String>()
    val webRequest: LiveData<String>
        get() = _webRequest

    fun onContactPress() {
        _webRequest.value = "https://lucodivo.github.io/about.html"
    }

    fun onSourcePress() {
        _webRequest.value = "https://github.com/Lucodivo/OpenGLScenes_Android"
    }

    fun onNightModeToggle(newState: Boolean) = scenesApp.setDarkMode(newState)
}