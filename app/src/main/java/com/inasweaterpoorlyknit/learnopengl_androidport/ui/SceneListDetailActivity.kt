package com.inasweaterpoorlyknit.learnopengl_androidport.ui

import android.os.Bundle
import androidx.activity.viewModels
import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.commit
import androidx.fragment.app.replace
import com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels.SceneListDetailViewModel
import com.inasweaterpoorlyknit.learnopengl_androidport.R

// TODO: Single Activity application
// HomeActivity encompasses two fragments
// 1) Scene List Fragment
// 2) Scene Fragment
// Fragments communicate through Activity view model?
// TODO: Pinch to zoom for Mandelbrot set

interface ListItemDataI {
    val imageResId: Int
    val displayTextResId: Int
    val descTextResId: Int
}

data class ListItemData(
    @DrawableRes override val imageResId: Int,
    @StringRes override val displayTextResId: Int,
    @StringRes override val descTextResId: Int
) : ListItemDataI

class SceneListDetailActivity : AppCompatActivity() {

    private val viewModel: SceneListDetailViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.fragment_container)

        viewModel.presentationMode.observe(this) { presentationMode ->
            supportFragmentManager.commit {
                setReorderingAllowed(true) // recommended for FragmentTransactions if able
                when (presentationMode) {
                    SceneListDetailViewModel.ListDetailPresentationMode.LIST -> replace<SceneListFragment>(R.id.fragment_container)
                    SceneListDetailViewModel.ListDetailPresentationMode.DETAIL -> replace<SceneFragment>(R.id.fragment_container)
                    SceneListDetailViewModel.ListDetailPresentationMode.FINISH -> super.onBackPressed()
                }
            }
        }

        window.apply {
            statusBarColor = resources.getColor(R.color.Grayscale0) // set
        }
    }

    override fun onBackPressed() {
        viewModel.onBackPress()
    }
}