package com.inasweaterpoorlyknit.learnopengl_androidport.ui

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.Icon
import androidx.compose.material.MaterialTheme
import androidx.compose.material.icons.rounded.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.fragment.app.Fragment
import androidx.fragment.app.activityViewModels
import com.inasweaterpoorlyknit.learnopengl_androidport.ui.theme.OpenGLScenesTheme
import com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels.SceneListDetailViewModel

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
                HomeList(activityViewModel.listItemData)
            }
        }
    }

    @Composable
    fun HomeList(itemDataList: List<ListItemDataI>) {
        OpenGLScenesTheme(this) {
            LazyColumn(
                contentPadding = PaddingValues(vertical = halfListPadding),
                modifier = Modifier
                    .background(color = MaterialTheme.colors.background)
                    .fillMaxSize()
            ) {
                item {
                    SettingsHeader()
                }
                items(itemDataList) { listItemData ->
                    HomeListItem(listItemData)
                }
            }
        }
    }

    @Composable
    fun SettingsHeader() {
        ScenesListItem(
            modifier = Modifier
                .clickable {
                    activityViewModel.onInfoPress()
                }
        ) {
            Icon(ScenesIcons.Info, contentDescription = "Info Icon", modifier = Modifier.padding(vertical = headerIconVertPadding))
        }
    }

    @Composable
    fun HomeListItem(listItemData: ListItemDataI) {
        ScenesListItem(
            modifier = Modifier
                .clickable {
                    activityViewModel.itemSelected(listItemData)
                }
        ) {
            Row {
                Image(
                    painter = painterResource(listItemData.imageResId),
                    contentDescription = stringResource(listItemData.descTextResId), // TODO: content descriptions are considerate
                    modifier = Modifier
                        .height(sceneListItemHeight)
                )

                ListItemText(text = stringResource(listItemData.displayTextResId),
                    modifier = Modifier.height(sceneListItemHeight))
            }
        }
    }

    @Preview
    @Composable
    fun HomeListPreview() {
        HomeList(SceneListDetailViewModel.listItemDataForComposePreview)
    }
}