package com.inasweaterpoorlyknit.learnopengl_androidport.activities

import android.opengl.GLSurfaceView
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import com.inasweaterpoorlyknit.learnopengl_androidport.scenes.InfiniteCubeScene

class InfiniteCubeActivity : AppCompatActivity() {

    private lateinit var gLView: GLSurfaceView

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Create a GLSurfaceView instance and set it
        // as the ContentView for this Activity.
        gLView = InfiniteCubeScene(this)
        setContentView(gLView)
    }

}
