package com.inasweaterpoorlyknit.learnopengl_androidport.activities

import android.content.res.Configuration
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import com.inasweaterpoorlyknit.learnopengl_androidport.scenes.InfiniteCubeScene

class InfiniteCubeActivity : AppCompatActivity() {

    private lateinit var scene: InfiniteCubeScene

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Create a GLSurfaceView instance and set it
        // as the ContentView for this Activity.
        scene = InfiniteCubeScene(this)
        setContentView(scene)
    }

    override fun onConfigurationChanged(newConfig: Configuration?) {
        super.onConfigurationChanged(newConfig)

        newConfig?.orientation?.let {
            scene.orientationChange(it)
        }
    }
}
