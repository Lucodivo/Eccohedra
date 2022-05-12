package com.inasweaterpoorlyknit.learnopengl_androidport.ui

import android.content.Intent
import android.os.Bundle
import androidx.activity.compose.setContent
import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.foundation.Image
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
import com.inasweaterpoorlyknit.learnopengl_androidport.R

// IDEA: Single Activity application
// HomeActivity encompasses two fragments
// 1) Scene List Fragment
// 2) Scene Fragment
// Fragments communicate through Activity view model?

class ProgramListItemData(@DrawableRes val imageResId: Int,
                          @StringRes val displayTextResId: Int,
                          @StringRes val descTextResId: Int,
                          val activityJavaClass: Class<*>)

class HomeActivity : AppCompatActivity() {

    companion object {
        // TODO: Acquire this list from view model?
        val programListItemsData = arrayOf(
            ProgramListItemData(imageResId = R.drawable.infinite_cube_thumbnail,
                                displayTextResId = R.string.infinite_cube_scene_title,
                                descTextResId = R.string.infinite_cube_thumbnail_description,
                                activityJavaClass = InfiniteCubeActivity::class.java),
            ProgramListItemData(imageResId = R.drawable.infinite_capsules_thumbnail,
                                displayTextResId = R.string.infinite_capsules_scene_title,
                                descTextResId = R.string.infinite_capsules_thumbnail_description,
                                activityJavaClass = InfiniteCapsulesActivity::class.java),
            ProgramListItemData(imageResId = R.drawable.mandelbrot_thumbnail,
                                displayTextResId = R.string.mandelbrot_scene_title,
                                descTextResId = R.string.mandelbrot_thumbnail_description,
                                activityJavaClass = MandelbrotActivity::class.java),
            ProgramListItemData(imageResId = R.drawable.menger_prison_thumbnail,
                                displayTextResId = R.string.menger_prison_scene_title,
                                descTextResId = R.string.menger_prison_thumbnail_description,
                                activityJavaClass = MengerPrisonActivity::class.java))
    }


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            programList(programListItemsData)
        }
    }

    @Composable
    fun programList(programs: Array<ProgramListItemData>) {
        LazyColumn {
            //item { } // optional header
            items(programs) { program ->
                ProgramListItem(programListItemData = program)
            }
        }
    }

    @Composable
    fun ProgramListItem(programListItemData: ProgramListItemData) {
        val rowItemHeight = 160.dp

        Surface(
            shape = MaterialTheme.shapes.medium,
            elevation = 1.dp,
            modifier = Modifier
                .padding(all = 8.dp)
                .fillMaxWidth()
                .clickable {
                    startActivity(Intent(this, programListItemData.activityJavaClass))
                }
        ) {
            Row {
                Image(
                    painter = painterResource(programListItemData.imageResId),
                    contentDescription = stringResource(programListItemData.descTextResId), // TODO: content descriptions are considerate
                    modifier = Modifier
                        .height(rowItemHeight)
                )
                //.clip(CircleShape))

                Spacer(modifier = Modifier.width(8.dp))

                Text(
                    text = stringResource(programListItemData.displayTextResId),
                    fontSize = 20.sp,
                    textAlign = TextAlign.Center,
                    modifier = Modifier
                        .height(rowItemHeight) // set min height
                        .wrapContentHeight(Alignment.CenterVertically) // center vertically
                        .padding(all = 10.dp)
                )
            }
        }
    }

    @Preview
    @Composable
    fun programListPreview() {
        programList(programListItemsData)
    }
}
