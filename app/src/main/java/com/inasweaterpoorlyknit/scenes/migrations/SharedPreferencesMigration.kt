package com.inasweaterpoorlyknit.scenes.migrations

import androidx.datastore.core.DataMigration
import com.inasweaterpoorlyknit.scenes.UserPreferences
import com.inasweaterpoorlyknit.scenes.copy
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MandelbrotScene
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MengerPrisonScene

/**
 * Migrates saved ids from [Int] to [String] types
 */
internal object SharedPreferencesMigration : DataMigration<UserPreferences> {

    override suspend fun cleanUp() = Unit

    override suspend fun migrate(currentData: UserPreferences): UserPreferences =
        currentData.copy {
            // Migrate to default values
            mengerIndex = MengerPrisonScene.DEFAULT_RESOLUTION_INDEX
            mandelbrotIndex = MandelbrotScene.DEFAULT_COLOR_INDEX

            // Mark migration as complete
            hasDoneSharedPreferencesMigration = true
        }

    override suspend fun shouldMigrate(currentData: UserPreferences): Boolean = !currentData.hasDoneSharedPreferencesMigration
}