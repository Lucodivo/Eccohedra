package com.inasweaterpoorlyknit.learnopengl_androidport.activities

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import kotlinx.android.synthetic.main.activity_home.*

class HomeActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_home)
        infiniteCubeThumbnail.setOnClickListener {
            val infiniteCubeIntent = Intent(this, InfiniteCubeActivity::class.java)
            startActivity(infiniteCubeIntent)
        }
        infiniteSphereThumbnail.setOnClickListener {
            val infiniteSphereIntent = Intent(this, InfiniteSphereActivity::class.java)
            startActivity(infiniteSphereIntent)
        }
    }
}
