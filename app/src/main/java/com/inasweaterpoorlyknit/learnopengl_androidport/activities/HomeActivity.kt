package com.inasweaterpoorlyknit.learnopengl_androidport.activities

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import kotlinx.android.synthetic.main.activity_home.*

class HomeActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_home)

        val programListItemsData = arrayOf(ProgramListItemData(R.drawable.infinite_cube_thumbnail, R.string.infinite_cube_scene_bottom_text),
                                    ProgramListItemData(R.drawable.infinite_capsules_thumbnail, R.string.infinite_capsules_scene_bottom_text),
                                    ProgramListItemData(R.drawable.mandelbrot_thumbnail, R.string.mandelbrot_scene_bottom_text),
                                    ProgramListItemData(R.drawable.menger_prison_thumbnail, R.string.menger_prison_scene_bottom_text))
        programRecyclerView.adapter = ProgramListItemAdapter(programListItemsData, ::onClick)
        programRecyclerView.layoutManager = LinearLayoutManager(this)
    }

    private fun onClick(imageResourceId: Int) {
        when(imageResourceId) {
            R.drawable.infinite_cube_thumbnail -> startActivity(Intent(this, InfiniteCubeActivity::class.java))
            R.drawable.infinite_capsules_thumbnail -> startActivity(Intent(this, InfiniteCapsulesActivity::class.java))
            R.drawable.mandelbrot_thumbnail -> startActivity(Intent(this, MandelbrotActivity::class.java))
            R.drawable.menger_prison_thumbnail -> startActivity(Intent(this, MengerPrisonActivity::class.java))
            else -> { /* DO NOTHING */}
        }
    }
}
