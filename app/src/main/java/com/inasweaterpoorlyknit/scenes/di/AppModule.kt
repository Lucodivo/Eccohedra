package com.inasweaterpoorlyknit.scenes.di

import android.content.Context
import com.inasweaterpoorlyknit.scenes.repositories.SharedPreferencesRepository
import com.inasweaterpoorlyknit.scenes.repositories.UserPreferencesDataStoreRepository
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object AppModule {
    @Singleton
    @Provides
    fun provideSharedPreferencesRepository(@ApplicationContext appContext: Context) = SharedPreferencesRepository(appContext)

    @Singleton
    @Provides
    fun provideUserPreferencesDataStoreRepository(@ApplicationContext appContext: Context) = UserPreferencesDataStoreRepository(appContext)
}