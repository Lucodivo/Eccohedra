package com.inasweaterpoorlyknit.scenes.graphics

import android.app.NativeActivity
import android.content.pm.ActivityInfo
import android.os.Bundle

/*
    NativeActivity is an Activity that is entirely controlled through native code (cpp)
    These native activities are linked with the corresponding cpp library code through the AndroidManifest
    In turn, these libraries are defined in cpp/CmakeLists.cpp and the corresponding cpp code
    This project uses native activities as a way of more easily porting code between Windows and Android
    with the hopes of pulling out a little extra performance along the way.
 */
open class NativeSceneActivity: NativeActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // Lock orientation for the duration of the scene
        requestedOrientation = when(orientation) {
            Orientation.Portrait -> ActivityInfo.SCREEN_ORIENTATION_PORTRAIT
            Orientation.Landscape -> ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
            Orientation.PortraitReverse -> ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT
            Orientation.LandscapeReverse -> ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        // release orientation to OS
        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED
    }
}

class TestNativeActivity: NativeSceneActivity()
class GateNativeActivity: NativeSceneActivity()