package com.inasweaterpoorlyknit.scenes.viewmodels

import com.inasweaterpoorlyknit.scenes.graphics.scenes.MandelbrotScene
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MengerPrisonScene
import com.inasweaterpoorlyknit.scenes.repositories.SharedPreferencesRepository
import com.airbnb.mvrx.MavericksState
import com.airbnb.mvrx.MavericksViewModel
import com.airbnb.mvrx.MavericksViewModelFactory
import com.airbnb.mvrx.hilt.AssistedViewModelFactory
import com.airbnb.mvrx.hilt.hiltMavericksViewModelFactory
import dagger.assisted.Assisted
import dagger.assisted.AssistedFactory
import dagger.assisted.AssistedInject

// TODO: Get the proper initial values for the Setting State somehow from the SharedPreferencesRepository
data class SettingsState(
    val mengerResolutionIndex: Int = MengerPrisonScene.DEFAULT_RESOLUTION_INDEX,
    val mandelbrotColorIndex: Int = MandelbrotScene.DEFAULT_COLOR_INDEX,
) : MavericksState {
    companion object {
        val mengerResolutionStrings: Array<String> = Array(MengerPrisonScene.resolutionFactorOptions.size) {
            "${(MengerPrisonScene.resolutionFactorOptions[it] * 100.0f).toInt()}%"
        }
        val mandelbrotColors: Array<String> = Array(MandelbrotScene.colors.size) { MandelbrotScene.colors[it].name }
        val websiteUrl = "https://lucodivo.github.io"
        val sourceUrl = "https://github.com/Lucodivo/ScenesMobile"
    }
}

class SettingsViewModel @AssistedInject constructor(
    @Assisted state: SettingsState,
    private val sharedPreferencesRepo: SharedPreferencesRepository,
) : MavericksViewModel<SettingsState>(state) {
    @AssistedFactory
    interface Factory : AssistedViewModelFactory<SettingsViewModel, SettingsState> {
        override fun create(state: SettingsState): SettingsViewModel
    }
    companion object : MavericksViewModelFactory<SettingsViewModel, SettingsState> by hiltMavericksViewModelFactory()

    init {
        setState{ copy(
            mengerResolutionIndex = sharedPreferencesRepo.getMengerSpongeResolutionIndex(),
            mandelbrotColorIndex = sharedPreferencesRepo.getMandelbrotColorIndex()
        )}
    }

    fun onMengerPrisonResolutionSelected(index: Int) {
        sharedPreferencesRepo.setMengerSpongeResolutionIndex(index)
        // TODO: get live updates from repository instead?
        setState{ copy(mengerResolutionIndex = index) }
    }
    fun onMandelbrotColorSelected(index: Int) {
        sharedPreferencesRepo.setMandelbrotColorIndex(index)
        // TODO: get live updates from repository instead?
        setState { copy(mandelbrotColorIndex = index) }
    }
}