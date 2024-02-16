package com.inasweaterpoorlyknit.scenes.ui

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.inasweaterpoorlyknit.scenes.R
import dagger.hilt.android.AndroidEntryPoint

@AndroidEntryPoint
class SceneListDetailActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.navigation_container)
    }
}