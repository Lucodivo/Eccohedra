package com.inasweaterpoorlyknit.scenes

import android.app.Application
import com.airbnb.mvrx.Mavericks
import com.google.android.material.color.DynamicColors
import dagger.hilt.android.HiltAndroidApp

@HiltAndroidApp
class ScenesApplication : Application() {
    override fun onCreate() {
        super.onCreate()
        DynamicColors.applyToActivitiesIfAvailable(this)
        Mavericks.initialize(this)
    }
}