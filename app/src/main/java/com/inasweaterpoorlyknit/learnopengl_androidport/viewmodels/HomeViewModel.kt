package com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import com.inasweaterpoorlyknit.learnopengl_androidport.ui.*

class HomeViewModel : ViewModel() {

    private val _startActivityRequest = MutableLiveData<Class<*>>()
    val startActivityRequest: LiveData<Class<*>>
        get() = _startActivityRequest

    companion object {
        private class ProgramListItemData(val listItemData: ListItemData,
                                          val activityJavaClass: Class<*>)
            : ListItemDataI by listItemData

        private val programListItemsData = listOf(
            ProgramListItemData(ListItemData(
                imageResId = R.drawable.infinite_cube_thumbnail,
                displayTextResId = R.string.infinite_cube_scene_title,
                descTextResId = R.string.infinite_cube_thumbnail_description),
                activityJavaClass = InfiniteCubeActivity::class.java),
            ProgramListItemData(ListItemData(
                imageResId = R.drawable.infinite_capsules_thumbnail,
                displayTextResId = R.string.infinite_capsules_scene_title,
                descTextResId = R.string.infinite_capsules_thumbnail_description),
                activityJavaClass = InfiniteCapsulesActivity::class.java),
            ProgramListItemData(ListItemData(
                imageResId = R.drawable.mandelbrot_thumbnail,
                displayTextResId = R.string.mandelbrot_scene_title,
                descTextResId = R.string.mandelbrot_thumbnail_description),
                activityJavaClass = MandelbrotActivity::class.java),
            ProgramListItemData(ListItemData(
                imageResId = R.drawable.menger_prison_thumbnail,
                displayTextResId = R.string.menger_prison_scene_title,
                descTextResId = R.string.menger_prison_thumbnail_description),
                activityJavaClass = MengerPrisonActivity::class.java)
        )

        val listItemDataForComposePreview: List<ListItemDataI>
            get() = programListItemsData
    }

    fun itemSelected(item: ListItemDataI) {
        _startActivityRequest.value = (item as ProgramListItemData).activityJavaClass
    }

    val listItemData: List<ListItemDataI>
        get() = programListItemsData
}