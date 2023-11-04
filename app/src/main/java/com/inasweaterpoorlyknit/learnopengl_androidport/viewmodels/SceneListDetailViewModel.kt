package com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels

import android.content.Context
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.GateNativeActivity
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.Scene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.TestNativeActivity
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.InfiniteCapsulesScene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.InfiniteCubeScene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MandelbrotScene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MengerPrisonScene
import com.inasweaterpoorlyknit.learnopengl_androidport.ui.InfoActivity
import com.inasweaterpoorlyknit.learnopengl_androidport.ui.ListItemData
import com.inasweaterpoorlyknit.learnopengl_androidport.ui.ListItemDataI

class SceneListDetailViewModel : ViewModel() {

    enum class ListDetailPresentationMode {
        LIST, DETAIL,
    }

    private val _startActivityRequest = MutableLiveData<Class<*>>()
    val startActivityRequest: LiveData<Class<*>>
        get() = _startActivityRequest

    private val _presentationMode = MutableLiveData<ListDetailPresentationMode>()
    val presentationMode: LiveData<ListDetailPresentationMode>
        get() = _presentationMode

    private val _finishTrigger = MutableLiveData<Unit>()
    val finishTrigger: LiveData<Unit>
        get() = _finishTrigger

    private lateinit var _sceneCreator : (Context) -> Scene
    val sceneCreator: (Context) -> Scene
        get() = _sceneCreator

    init {
        _presentationMode.value = ListDetailPresentationMode.LIST

        //debugScene(::MandelbrotScene)
        //debugNativeScene(BlueSceneNativeActivity::class.java)
    }

    // NOTE: Call this in init() to jump right into a scene for debugging
    fun debugScene(sceneConstructor: (Context) -> Scene) {
        _sceneCreator = sceneConstructor
        _presentationMode.value = ListDetailPresentationMode.DETAIL
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

        private val programListItemsData = listOf(
            KotlinProgramListItemData(ListItemData(
                imageResId = R.drawable.infinite_cube_2720_1440,
                displayTextResId = R.string.infinite_cube_scene_title,
                descTextResId = R.string.infinite_cube_thumbnail_description),
                sceneCreator = ::InfiniteCubeScene),
            KotlinProgramListItemData(ListItemData(
                imageResId = R.drawable.infinite_capsules_2720_1440,
                displayTextResId = R.string.infinite_capsules_scene_title,
                descTextResId = R.string.infinite_capsules_thumbnail_description),
                sceneCreator = ::InfiniteCapsulesScene),
            KotlinProgramListItemData(ListItemData(
                imageResId = R.drawable.mandelbrot_2720_1440,
                displayTextResId = R.string.mandelbrot_scene_title,
                descTextResId = R.string.mandelbrot_thumbnail_description),
                sceneCreator = ::MandelbrotScene),
            KotlinProgramListItemData(ListItemData(
                imageResId = R.drawable.menger_prison_2720_1440,
                displayTextResId = R.string.menger_prison_scene_title,
                descTextResId = R.string.menger_prison_thumbnail_description),
                sceneCreator = ::MengerPrisonScene),
            // TODO: Update image and text
            NativeProgramListItemData(ListItemData(
                imageResId = R.drawable.blue_debug,
                displayTextResId = R.string.debug_string_id,
                descTextResId = R.string.debug_string_id),
                nativeActivity = GateNativeActivity::class.java),
            NativeProgramListItemData(ListItemData(
                imageResId = R.drawable.red_debug,
                displayTextResId = R.string.debug_string_id,
                descTextResId = R.string.debug_string_id),
                nativeActivity = TestNativeActivity::class.java)
        )

        val listItemDataForComposePreview: List<ListItemDataI>
            get() = programListItemsData
    }

    fun itemSelected(item: ListItemDataI) {
        // NOTE: Scene creator is a regular value, make sure to set it before any sort of LiveData to avoid any type of race conditions
        when(item) {
            is KotlinProgramListItemData -> {
                _sceneCreator = item.sceneCreator
                _presentationMode.value = ListDetailPresentationMode.DETAIL
            }
            is NativeProgramListItemData -> {
                _startActivityRequest.value = item.nativeActivity
            }
        }
    }

    fun onBackPress() {
        _presentationMode.value?.let {
            when (it) {
                ListDetailPresentationMode.LIST -> { _finishTrigger.value = Unit }
                ListDetailPresentationMode.DETAIL -> { _presentationMode.value = ListDetailPresentationMode.LIST }
            }
        }
    }

    fun onInfoPress() {
        _startActivityRequest.value = InfoActivity::class.java
    }

    val listItemData: List<ListItemDataI>
        get() = programListItemsData
}