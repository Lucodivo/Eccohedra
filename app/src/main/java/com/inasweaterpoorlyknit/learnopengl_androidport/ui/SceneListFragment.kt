package com.inasweaterpoorlyknit.learnopengl_androidport.ui

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
import androidx.compose.material.icons.rounded.Info
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
            Icon(ScenesIcons.Settings, contentDescription = "Info Icon", modifier = Modifier.padding(vertical = headerIconVertPadding))
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

    @Preview
    @Composable
    fun HomeListPreview() {
        HomeList(SceneListDetailViewModel.listItemDataForComposePreview)
    }
}