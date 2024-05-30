package com.inasweaterpoorlyknit.scenes

import android.app.Application
import com.airbnb.mvrx.Mavericks
import dagger.hilt.android.HiltAndroidApp

@HiltAndroidApp
class ScenesApplication : Application() {
    override fun onCreate() {
        super.onCreate()
        Mavericks.initialize(this)
    }
}