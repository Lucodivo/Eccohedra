package com.inasweaterpoorlyknit.scenes.repositories

import android.content.Context
import com.inasweaterpoorlyknit.scenes.UserPreferences
import com.inasweaterpoorlyknit.scenes.userPreferencesDataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

class UserPreferencesDataStoreRepository(context: Context) {
    private val userPreferencesDataStore = context.userPreferencesDataStore

    val userPreferences: Flow<UserPreferences> = userPreferencesDataStore.data
    val mengerIndex: Flow<Int> = userPreferences.map { it.mengerIndex }
    val mandelbrotIndex: Flow<Int> = userPreferences.map { it.mandelbrotIndex }

    suspend fun setMengerIndex(index: Int) {
        userPreferencesDataStore.updateData { currentSettings ->
            currentSettings.toBuilder()
                .setMengerIndex(index)
                .build()
        }
    }

    suspend fun setMandelbrotIndex(index: Int) {
        userPreferencesDataStore.updateData { currentSettings ->
            currentSettings.toBuilder()
                .setMandelbrotIndex(index)
                .build()
        }
    }
}