package com.inasweaterpoorlyknit.learnopengl_androidport.ui

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.commit
import androidx.fragment.app.replace
import com.inasweaterpoorlyknit.learnopengl_androidport.R

class SettingsActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.fragment_container)
        supportFragmentManager.commit {
            setReorderingAllowed(true) // recommended for FragmentTransactions if able
            replace<SettingsFragment>(R.id.fragment_container)
        }
    }
}