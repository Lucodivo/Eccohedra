package com.inasweaterpoorlyknit.learnopengl_androidport.ui

import android.content.Intent
import android.content.pm.ActivityInfo
import android.os.Bundle
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.commit
import androidx.fragment.app.replace
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.Orientation
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.orientation
import com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels.SceneListDetailViewModel

class SceneListDetailActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.navigation_container)
    }
}
//class SceneListDetailActivity : AppCompatActivity() {
//
//    private val viewModel: SceneListDetailViewModel by viewModels()
//
//    override fun onCreate(savedInstanceState: Bundle?) {
//        super.onCreate(savedInstanceState)
//
//        setContentView(R.layout.fragment_container)
//
//        viewModel.presentationMode.observe(this) { presentationMode ->
//            supportFragmentManager.commit {
//                setReorderingAllowed(true) // recommended for FragmentTransactions if able
//                when (presentationMode) {
//                    ListDetailPresentationMode.LIST -> {
//                        // release orientation to OS
//                        requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED
//                        replace<SceneListFragment>(R.id.fragment_container)
//                    }
//                    ListDetailPresentationMode.DETAIL -> {
//                        // Lock orientation for the duration of the scene
//                        requestedOrientation = when(orientation) {
//                            Orientation.Portrait -> ActivityInfo.SCREEN_ORIENTATION_PORTRAIT
//                            Orientation.Landscape -> ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
//                            Orientation.PortraitReverse -> ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT
//                            Orientation.LandscapeReverse -> ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE
//                        }
//                        replace<SceneFragment>(R.id.fragment_container)
//                    }
//                    null -> { throw IllegalStateException("${ListDetailPresentationMode::class.java} contained a null value.") }
//                }
//            }
//        }
//
//        viewModel.finishTrigger.observe(this) {
//            super.onBackPressedDispatcher.onBackPressed()
//        }
//
//        viewModel.startActivityRequest.observe(this) {
//            val startActivityIntent = Intent(this, it)
//            startActivity(startActivityIntent)
//        }
//
//        window.apply {
//            statusBarColor = getColor(R.color.Grayscale0) // set
//        }
//    }
//
//    override fun onBackPressed() {
//        viewModel.onBackPress()
//    }
//}
