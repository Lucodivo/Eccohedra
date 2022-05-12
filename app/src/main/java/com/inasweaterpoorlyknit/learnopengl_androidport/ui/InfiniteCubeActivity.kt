package com.inasweaterpoorlyknit.learnopengl_androidport.ui

import android.content.res.Configuration
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.InfiniteCubeScene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.SceneSurfaceView
import com.inasweaterpoorlyknit.learnopengl_androidport.utils.hideSystemUI

class InfiniteCubeActivity : AppCompatActivity() {

    private lateinit var scene: SceneSurfaceView

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        scene = SceneSurfaceView(this, InfiniteCubeScene(this))
        setContentView(scene)
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)

        newConfig.orientation.let {
            scene.orientationChange(it)
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) hideSystemUI()
    }
}
