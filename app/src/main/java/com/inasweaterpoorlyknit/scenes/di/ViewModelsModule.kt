package com.inasweaterpoorlyknit.scenes.di

import android.content.Context
import com.airbnb.mvrx.hilt.AssistedViewModelFactory
import com.airbnb.mvrx.hilt.MavericksViewModelComponent
import com.airbnb.mvrx.hilt.ViewModelKey
import com.inasweaterpoorlyknit.scenes.repositories.SharedPreferencesRepository
import com.inasweaterpoorlyknit.scenes.viewmodels.SettingsViewModel
import dagger.Binds
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.multibindings.IntoMap
import javax.inject.Singleton

@Module
@InstallIn(MavericksViewModelComponent::class)
interface ViewModelsModule {

    @Binds
    @IntoMap
    @ViewModelKey(SettingsViewModel::class)
    fun infoViewModelFactory(factory: SettingsViewModel.Factory): AssistedViewModelFactory<*, *>
}