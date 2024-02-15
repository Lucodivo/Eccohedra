package com.inasweaterpoorlyknit.scenes.viewmodels

import com.inasweaterpoorlyknit.scenes.graphics.scenes.MandelbrotScene
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MengerPrisonScene
import com.airbnb.mvrx.MavericksState
import com.airbnb.mvrx.MavericksViewModel
import com.airbnb.mvrx.MavericksViewModelFactory
import com.airbnb.mvrx.hilt.AssistedViewModelFactory
import com.airbnb.mvrx.hilt.hiltMavericksViewModelFactory
import com.inasweaterpoorlyknit.scenes.repositories.UserPreferencesDataStoreRepository
import dagger.assisted.Assisted
import dagger.assisted.AssistedFactory
import dagger.assisted.AssistedInject

// TODO: Get the proper initial values for the Setting State somehow from the SharedPreferencesRepository
data class SettingsState(
    val mengerResolutionIndex: Int = MengerPrisonScene.DEFAULT_RESOLUTION_INDEX,
    val mandelbrotColorIndex: Int = MandelbrotScene.DEFAULT_COLOR_INDEX,
) : MavericksState {
    companion object {
        val mengerResolutionStrings = MengerPrisonScene.resolutionFactorOptions.map{ "${(it * 100.0f).toInt()}%" }
        val mandelbrotColors = MandelbrotScene.colors.map{ it.name }
        const val WEBSITE_URL = "https://lucodivo.github.io"
        const val SOURCE_URL = "https://github.com/Lucodivo/ScenesMobile"
    }
}

class SettingsViewModel @AssistedInject constructor(
    @Assisted state: SettingsState,
    private val userPreferencesRepo: UserPreferencesDataStoreRepository,
) : MavericksViewModel<SettingsState>(state) {
    @AssistedFactory
    interface Factory : AssistedViewModelFactory<SettingsViewModel, SettingsState> {
        override fun create(state: SettingsState): SettingsViewModel
    }
    companion object : MavericksViewModelFactory<SettingsViewModel, SettingsState> by hiltMavericksViewModelFactory()

    init {
        userPreferencesRepo.userPreferences.setOnEach { value ->
            copy(
                mengerResolutionIndex = value.mengerIndex,
                mandelbrotColorIndex = value.mandelbrotIndex
            )
        }
    }

    suspend fun onMengerPrisonResolutionSelected(index: Int) {
        userPreferencesRepo.setMengerIndex(index)
    }
    suspend fun onMandelbrotColorSelected(index: Int) {
        userPreferencesRepo.setMandelbrotIndex(index)
    }
}