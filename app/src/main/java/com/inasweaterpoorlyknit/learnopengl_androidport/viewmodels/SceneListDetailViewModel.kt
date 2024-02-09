package com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels

import android.content.Context
import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.GateNativeActivity
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.Scene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.InfiniteCapsulesScene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.InfiniteCubeScene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MandelbrotScene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MengerPrisonScene

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

class SceneListDetailViewModel : ViewModel() {

    private val _startActivityRequest = MutableLiveData<Class<*>>()
    val startActivityRequest: LiveData<Class<*>>
        get() = _startActivityRequest

    private lateinit var _sceneCreator : (Context) -> Scene
    val sceneCreator: (Context) -> Scene
        get() = _sceneCreator

    init {
        //debugScene(::InfiniteCapsulesScene)
        //debugNativeScene(BlueSceneNativeActivity::class.java)
    }

    // NOTE: Call this in init() to jump right into a scene for debugging
    fun debugScene(sceneConstructor: (Context) -> Scene) {
        _sceneCreator = sceneConstructor
    }

    // NOTE: Call this in init() to jump right into a scene for debugging
    fun debugNativeScene(nativeScene: Class<*>) {
        _startActivityRequest.value = nativeScene
    }

    companion object {
        private class KotlinProgramListItemData(val listItemData: ListItemData,
                                                val sceneCreator: (context: Context) -> Scene)
            : ListItemDataI by listItemData

        private class NativeProgramListItemData(val listItemData: ListItemData,
                                                val nativeActivity: Class<*>)
            : ListItemDataI by listItemData

        private val sceneListItemData = listOf(
            KotlinProgramListItemData(
                ListItemData(
                imageResId = R.drawable.infinite_cube_2720_1440,
                displayTextResId = R.string.infinite_cube_scene_title,
                descTextResId = R.string.infinite_cube_thumbnail_description),
                sceneCreator = ::InfiniteCubeScene),
            KotlinProgramListItemData(
                ListItemData(
                imageResId = R.drawable.infinite_capsules_2720_1440,
                displayTextResId = R.string.infinite_capsules_scene_title,
                descTextResId = R.string.infinite_capsules_thumbnail_description),
                sceneCreator = ::InfiniteCapsulesScene),
            KotlinProgramListItemData(
                ListItemData(
                imageResId = R.drawable.mandelbrot_2720_1440,
                displayTextResId = R.string.mandelbrot_scene_title,
                descTextResId = R.string.mandelbrot_thumbnail_description),
                sceneCreator = ::MandelbrotScene),
            KotlinProgramListItemData(
                ListItemData(
                imageResId = R.drawable.menger_prison_2720_1440,
                displayTextResId = R.string.menger_prison_scene_title,
                descTextResId = R.string.menger_prison_thumbnail_description),
                sceneCreator = ::MengerPrisonScene),
        )
        val sceneListItems: List<ListItemDataI>
            get() = sceneListItemData

        private val nativeSceneListItemData = listOf(
            NativeProgramListItemData(
                ListItemData(
                    imageResId = R.drawable.gate_2720_1440,
                    displayTextResId = R.string.gate_scene_title,
                    descTextResId = R.string.gate_thumbnail_description),
                nativeActivity = GateNativeActivity::class.java),
        )
        val nativeSceneListItems: List<ListItemDataI>
            get() = nativeSceneListItemData
    }

    fun itemSelected(item: ListItemDataI) {
        when(item) {
            is KotlinProgramListItemData -> {
                // NOTE: Scene creator is a regular value, make sure to set it before any sort of LiveData to avoid any type of race conditions
                _sceneCreator = item.sceneCreator
            }
            is NativeProgramListItemData -> {
                // TODO
            }
        }
    }
}