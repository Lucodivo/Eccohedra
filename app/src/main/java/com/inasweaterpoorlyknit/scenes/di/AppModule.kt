package com.inasweaterpoorlyknit.scenes.di

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.core.DataStoreFactory
import androidx.datastore.dataStoreFile
import com.inasweaterpoorlyknit.scenes.di.InjectDispatcher.IO
import com.inasweaterpoorlyknit.scenes.UserPreferences
import com.inasweaterpoorlyknit.scenes.UserPreferencesSerializer
import com.inasweaterpoorlyknit.scenes.migrations.SharedPreferencesMigration
import com.inasweaterpoorlyknit.scenes.repositories.UserPreferencesDataStoreRepository
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import kotlinx.coroutines.CoroutineDispatcher
import kotlinx.coroutines.CoroutineScope
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object AppModule {
    @Provides
    @Singleton
    internal fun providesUserPreferencesDataStore(
        @ApplicationContext context: Context,
        @Dispatcher(IO) ioDispatcher: CoroutineDispatcher,
        @ApplicationScope scope: CoroutineScope,
        userPreferencesSerializer: UserPreferencesSerializer,
    ): DataStore<UserPreferences> =
        DataStoreFactory.create(
            serializer = userPreferencesSerializer,
            scope = CoroutineScope(scope.coroutineContext + ioDispatcher),
            migrations = listOf(
                SharedPreferencesMigration,
            ),
        ) {
            context.dataStoreFile("user_preferences.pb")
        }

    @Provides
    @Singleton
    fun provideUserPreferencesDataStoreRepository(dataStore: DataStore<UserPreferences>) = UserPreferencesDataStoreRepository(dataStore)
}