package com.inasweaterpoorlyknit.scenes.viewmodels

import android.content.Context
import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import com.airbnb.mvrx.MavericksState
import com.airbnb.mvrx.MavericksViewModel
import com.inasweaterpoorlyknit.scenes.R
import com.inasweaterpoorlyknit.scenes.graphics.RotationSensorHelper
import com.inasweaterpoorlyknit.scenes.graphics.Scene
import com.inasweaterpoorlyknit.scenes.graphics.orientation
import com.inasweaterpoorlyknit.scenes.graphics.scenes.InfiniteCapsulesScene
import com.inasweaterpoorlyknit.scenes.graphics.scenes.InfiniteCubeScene
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MandelbrotScene
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MengerPrisonScene
import com.inasweaterpoorlyknit.scenes.repositories.UserPreferencesDataStoreRepository

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

class KotlinSceneListItemData(
    private val listItemData: ListItemDataI,
    val sceneCreator: (context: Context, UserPreferencesDataStoreRepository, RotationSensorHelper) -> Scene,
) : ListItemDataI by listItemData

data class SceneListData(val selectedSceneIndex: Int = 0) : MavericksState {
    val sceneCreator : (Context, UserPreferencesDataStoreRepository, RotationSensorHelper) -> Scene
        get() {
            val index = if(selectedSceneIndex > 0 && selectedSceneIndex < kotlinScenesList.size) selectedSceneIndex else 0
            return (kotlinScenesList[index] as KotlinSceneListItemData).sceneCreator
        }

   companion object {
       val kotlinScenesList: List<ListItemDataI> = listOf(
           KotlinSceneListItemData(
               ListItemData(
                   imageResId = R.drawable.infinite_cube_2720_1440,
                   displayTextResId = R.string.infinite_cube_scene_title,
                   descTextResId = R.string.infinite_cube_thumbnail_description
               )
           ) { context, _, _ ->
               InfiniteCubeScene(context, context.resources, context.orientation)
           },
           KotlinSceneListItemData(
               ListItemData(
                   imageResId = R.drawable.infinite_capsules_2720_1440,
                   displayTextResId = R.string.infinite_capsules_scene_title,
                   descTextResId = R.string.infinite_capsules_thumbnail_description
               )
           ) { context, _, rotationSensorHelper ->
               InfiniteCapsulesScene(rotationSensorHelper, context.resources, context.orientation)
           },
           KotlinSceneListItemData(
               ListItemData(
                   imageResId = R.drawable.mandelbrot_2720_1440,
                   displayTextResId = R.string.mandelbrot_scene_title,
                   descTextResId = R.string.mandelbrot_thumbnail_description
               )
           ) { context, userPreferencesRepo, _ ->
               MandelbrotScene(context, userPreferencesRepo, context.resources)
           },
           KotlinSceneListItemData(
               ListItemData(
                   imageResId = R.drawable.menger_prison_2720_1440,
                   displayTextResId = R.string.menger_prison_scene_title,
                   descTextResId = R.string.menger_prison_thumbnail_description
               )
           ) { context, userPreferencesRepo, rotationSensorHelper ->
               MengerPrisonScene(rotationSensorHelper, userPreferencesRepo, context.resources, context.orientation)
           },
       )
       val gateScene = ListItemData(
           imageResId = R.drawable.gate_2720_1440,
           displayTextResId = R.string.gate_scene_title,
           descTextResId = R.string.gate_thumbnail_description
       )
   }
}

class SceneListDetailViewModel(state: SceneListData) : MavericksViewModel<SceneListData>(state) {
    fun itemSelected(itemIndex: Int) {
        // NOTE: Scene creator is a regular value, make sure to set it before any sort of LiveData to avoid any type of race conditions
        setState { copy(selectedSceneIndex = itemIndex) }
    }
}