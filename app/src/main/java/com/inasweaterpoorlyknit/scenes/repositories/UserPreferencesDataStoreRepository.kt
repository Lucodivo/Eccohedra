package com.inasweaterpoorlyknit.scenes.repositories

import androidx.datastore.core.DataStore
import com.inasweaterpoorlyknit.scenes.UserPreferences
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

class UserPreferencesDataStoreRepository(private val dataStore: DataStore<UserPreferences>) {

    val userPreferences: Flow<UserPreferences> = dataStore.data
    val mengerIndex: Flow<Int> = userPreferences.map { it.mengerIndex }
    val mandelbrotIndex: Flow<Int> = userPreferences.map { it.mandelbrotIndex }

    suspend fun setMengerIndex(index: Int) {
        dataStore.updateData { currentSettings ->
            currentSettings.toBuilder()
                .setMengerIndex(index)
                .build()
        }
    }

    suspend fun setMandelbrotIndex(index: Int) {
        dataStore.updateData { currentSettings ->
            currentSettings.toBuilder()
                .setMandelbrotIndex(index)
                .build()
        }
    }
}