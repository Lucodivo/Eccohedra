package com.inasweaterpoorlyknit.learnopengl_androidport.ui

import android.os.Bundle
import androidx.activity.compose.setContent
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material.MaterialTheme
import androidx.compose.material.Text
import androidx.compose.material.icons.rounded.ContactPage
import androidx.compose.material.icons.rounded.OpenInNew
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MandelbrotScene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MengerPrisonScene
import com.inasweaterpoorlyknit.learnopengl_androidport.ui.theme.OpenGLScenesTheme
import com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels.InfoViewModel

class InfoActivity : AppCompatActivity() {

    private val viewModel: InfoViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            InfoList(viewModel.getDarkMode(), viewModel.getMengerSpongeResolutionIndex(), viewModel.getMandelbrotColorIndex())
        }

        viewModel.webRequest.observe(this) { openWebPage(it) }
    }

    @Preview
    @Composable
    fun InfoListPreview() {
        InfoList(true, MengerPrisonScene.defaultResolutionIndex, MandelbrotScene.defaultColorIndex)
    }

    @Composable
    fun InfoList(initDarkMode: Boolean, initMengerResolutionIndex: Int, initMandelbrotColorIndex: Int) {
        OpenGLScenesTheme(this) {
            LazyColumn(
                contentPadding = PaddingValues(vertical = halfListPadding),
                modifier = Modifier
                    .background(color = MaterialTheme.colors.background)
                    .fillMaxSize()
            ) {
                // About Header
                item {
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = "Info",
                        color = MaterialTheme.colors.onBackground,
                        fontSize = listItemFontSize,
                        textAlign = TextAlign.Center,
                        modifier = Modifier
                            .padding(all = listItemTextPadding)
                            .fillMaxWidth()
                    )
                }

                // Scenes Title
                item {
                    ScenesListItem {
                        ListItemText(text = "Scenes", color = MaterialTheme.colors.onPrimary, modifier = Modifier.background(color = MaterialTheme.colors.primary))
                    }
                }

                // Author Contact
                item {
                    ScenesListItem {
                        ListItemTextWithLeftIcon("Connor Alexander Haskins", icon = ScenesIcons.ContactPage,
                            modifier = Modifier.clickable { viewModel.onContactPress() })
                    }
                }

                // Source Code
                item {
                    ScenesListItem {
                        ListItemTextWithRightIcon(text = "Source", icon = ScenesIcons.OpenInNew,
                            modifier = Modifier.clickable { viewModel.onSourcePress() })
                    }
                }

                // Settings Header
                item {
                    Spacer(modifier = Modifier.height(40.dp))
                    Text(
                        text = "Settings",
                        color = MaterialTheme.colors.onBackground,
                        fontSize = listItemFontSize,
                        textAlign = TextAlign.Center,
                        modifier = Modifier
                            .padding(all = listItemTextPadding)
                            .fillMaxWidth()
                    )
                }

                // Dark Mode
                item {
                    ScenesListItem {
                        ListItemSwitch("Dark Mode 🌙", initDarkMode) {
                            viewModel.onNightModeToggle(it)
                        }
                    }
                }

                // Menger Prison Resolution
                item {
                    ScenesListItem {
                        ListItemDropdown(
                            titleText = "Menger Sponge Resolution",
                            items = InfoViewModel.mengerPrisonResolutions,
                            initSelectedIndex = initMengerResolutionIndex,
                            selectedDecorationText = "🎞"
                        ){ viewModel.onMengerPrisonResolutionSelected(it) }
                    }
                }

                // Mandelbrot Color
                item {
                    ScenesListItem {
                        ListItemDropdown(
                            titleText = "Mandelbrot Color",
                            items = InfoViewModel.mandelbrotColors,
                            initSelectedIndex = initMandelbrotColorIndex,
                            selectedDecorationText = "🎨"
                        ){ viewModel.onMandelbrotColorSelected(it) }
                    }
                }
            }
        }
    }
}