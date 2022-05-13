package com.inasweaterpoorlyknit.learnopengl_androidport.ui

import android.content.Intent
import android.os.Bundle
import androidx.activity.compose.setContent
import androidx.activity.viewModels
import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.MaterialTheme
import androidx.compose.material.Surface
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.inasweaterpoorlyknit.learnopengl_androidport.ui.theme.OpenGLScenesTheme
import com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels.HomeViewModel

// TODO: Single Activity application
// HomeActivity encompasses two fragments
// 1) Scene List Fragment
// 2) Scene Fragment
// Fragments communicate through Activity view model?
// TODO: Pinch to zoom for Mandelbrot set

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

class HomeActivity : AppCompatActivity() {

    private val listItemFontSize = 20.sp
    private val listItemHeight = 200.dp
    private val listPadding = 8.dp
    private val halfListPadding = listPadding / 2

    private val viewModel: HomeViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        viewModel.startActivityRequest.observe(this) { activity ->
            startActivity(Intent(this, activity))
        }
        setContent {
            HomeList(viewModel.listItemData)
        }
    }

    @Composable
    fun HomeList(itemDataList: List<ListItemDataI>) {
        OpenGLScenesTheme {
            LazyColumn(
                contentPadding = PaddingValues(vertical = halfListPadding),
                modifier = Modifier
                    .background(color = MaterialTheme.colors.background)
                    .fillMaxSize()
            ) {
                //item { } // optional header
                items(itemDataList) { listItemData ->
                    HomeListItem(listItemData)
                }
            }
        }
    }

    @Composable
    fun HomeListItem(listItemData: ListItemDataI) {
        Surface(
            elevation = 1.dp,
            shape = MaterialTheme.shapes.large,
            modifier = Modifier
                .padding(horizontal = listPadding, vertical = halfListPadding)
                .fillMaxWidth()
                .clickable {
                    viewModel.itemSelected(listItemData)
                }
        ) {
            Row {
                Image(
                    painter = painterResource(listItemData.imageResId),
                    contentDescription = stringResource(listItemData.descTextResId), // TODO: content descriptions are considerate
                    modifier = Modifier
                        .height(listItemHeight)
//                        .clip(CircleShape)
                )

                Text(
                    text = stringResource(listItemData.displayTextResId),
                    fontSize = listItemFontSize,
                    textAlign = TextAlign.Center,
                    modifier = Modifier
                        .height(listItemHeight) // set min height
                        .wrapContentHeight(Alignment.CenterVertically) // center vertically
                        .padding(all = 10.dp)
                )
            }
        }
    }

    @Preview
    @Composable
    fun HomeListPreview() {
        HomeList(HomeViewModel.listItemDataForComposePreview)
    }
}
