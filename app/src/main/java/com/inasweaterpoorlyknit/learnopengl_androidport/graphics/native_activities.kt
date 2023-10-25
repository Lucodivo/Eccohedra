package com.inasweaterpoorlyknit.learnopengl_androidport.graphics

import android.app.NativeActivity

/*
    NativeActivity is an Activity that is entirely controlled through native code (cpp)
    These native activities are linked with the corresponding cpp library code through the AndroidManifest
    In turn, these libraries are defined in cpp/CmakeLists.cpp and the corresponding cpp code
    This project uses native activities as a way of more easily porting code between Windows and Android
    with the hopes of pulling out a little extra performance along the way.
 */
class TestNativeActivity: NativeActivity()
class GateNativeActivity: NativeActivity()