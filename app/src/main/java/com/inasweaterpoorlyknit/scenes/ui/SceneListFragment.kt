package com.inasweaterpoorlyknit.scenes.ui

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.rounded.Settings
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.fragment.app.Fragment
import androidx.fragment.app.activityViewModels
import androidx.navigation.fragment.findNavController
import com.inasweaterpoorlyknit.scenes.ui.theme.OpenGLScenesTheme
import com.inasweaterpoorlyknit.scenes.viewmodels.ListItemDataI
import com.inasweaterpoorlyknit.scenes.viewmodels.SceneListDetailViewModel

class SceneListFragment: Fragment() {
    private val headerIconVertPadding = 8.dp
    private val sceneListItemHeight = 200.dp

    private val activityViewModel: SceneListDetailViewModel by activityViewModels()

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        return ComposeView(requireContext()).apply {
            setContent {
                SceneList()
            }
        }
    }

    @Preview
    @Composable
    fun SceneList() {
        OpenGLScenesTheme {
            LazyColumn(
                contentPadding = PaddingValues(vertical = halfListPadding),
                modifier = Modifier
                    .background(color = MaterialTheme.colorScheme.background)
                    .fillMaxSize()
            ) {
                item {
                    SettingsHeader()
                }
                items(SceneListDetailViewModel.sceneListItems) { listItemData ->
                    SceneListItem(listItemData) {
                        activityViewModel.itemSelected(listItemData)
                        val directions = SceneListFragmentDirections.actionSceneListFragmentToSceneFragment()
                        findNavController().navigate(directions)
                    }
                }
                items(SceneListDetailViewModel.nativeSceneListItems) { listItemData ->
                    SceneListItem(listItemData) {
                        activityViewModel.itemSelected(listItemData)
                        val directions = SceneListFragmentDirections.actionSceneListFragmentToGateNativeActivity()
                        findNavController().navigate(directions)
                    }
                }
            }
        }
    }

    @Composable
    fun SettingsHeader() {
        ScenesListItem(
            modifier = Modifier
                .clickable {
                    val directions = SceneListFragmentDirections.actionSceneListFragmentToSettingsFragment()
                    findNavController().navigate(directions)
                }
        ) {
            Icon(ScenesIcons.Settings, contentDescription = "Info Icon", modifier = Modifier.padding(vertical = headerIconVertPadding))
        }
    }

    @Composable
    fun SceneListItem(listItemData: ListItemDataI, onClick: () -> Unit) {
        ScenesListItem(
            modifier = Modifier
                .clickable {
                    onClick()
                }
        ) {
                Image(
                    painter = painterResource(listItemData.imageResId),
                    contentDescription = stringResource(listItemData.descTextResId),
                    contentScale = ContentScale.FillWidth,
                    modifier = Modifier
                        .heightIn(min = 0.dp, max = sceneListItemHeight)
                        .fillMaxWidth()
                )
        }
    }
}