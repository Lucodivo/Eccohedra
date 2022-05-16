package com.inasweaterpoorlyknit.learnopengl_androidport.ui

import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.activityViewModels
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.SceneSurfaceView
import com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels.SceneListDetailViewModel

class SceneFragment: Fragment() {

  private val activityViewModel: SceneListDetailViewModel by activityViewModels()

  private lateinit var sceneSurfaceView: SceneSurfaceView

  override fun onAttach(context: Context) {
    super.onAttach(context)
    requireActivity().hideSystemUI()
  }

  override fun onDetach() {
    super.onDetach()
    requireActivity().showSystemUI()
  }

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)

    val context = requireContext()
    sceneSurfaceView = SceneSurfaceView(context, activityViewModel.sceneCreator(context));
  }

  override fun onCreateView(
    inflater: LayoutInflater, container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return sceneSurfaceView
  }
}