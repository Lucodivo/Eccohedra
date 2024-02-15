package com.inasweaterpoorlyknit.scenes.di

import com.airbnb.mvrx.hilt.AssistedViewModelFactory
import com.airbnb.mvrx.hilt.MavericksViewModelComponent
import com.airbnb.mvrx.hilt.ViewModelKey
import com.inasweaterpoorlyknit.scenes.viewmodels.SettingsViewModel
import dagger.Binds
import dagger.Module
import dagger.hilt.InstallIn
import dagger.multibindings.IntoMap

@Module
@InstallIn(MavericksViewModelComponent::class)
interface ViewModelsModule {

    @Binds
    @IntoMap
    @ViewModelKey(SettingsViewModel::class)
    fun infoViewModelFactory(factory: SettingsViewModel.Factory): AssistedViewModelFactory<*, *>
}