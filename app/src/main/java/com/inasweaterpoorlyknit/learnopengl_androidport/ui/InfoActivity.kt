package com.inasweaterpoorlyknit.learnopengl_androidport.ui

import android.os.Bundle
import androidx.activity.compose.setContent
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material.DropdownMenu
import androidx.compose.material.DropdownMenuItem
import androidx.compose.material.MaterialTheme
import androidx.compose.material.Text
import androidx.compose.material.icons.rounded.ContactPage
import androidx.compose.material.icons.rounded.OpenInNew
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MengerPrisonScene
import com.inasweaterpoorlyknit.learnopengl_androidport.ui.theme.OpenGLScenesTheme
import com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels.InfoViewModel

class InfoActivity : AppCompatActivity() {

    private val viewModel: InfoViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            InfoList(viewModel.getDarkMode(), viewModel.getMengerSpongeResolutionIndex())
        }

        viewModel.webRequest.observe(this) { openWebPage(it) }
    }

    @Preview
    @Composable
    fun InfoListPreview() {
        InfoList(true, MengerPrisonScene.defaultResolutionIndex)
    }

    @Composable
    fun InfoList(initDarkMode: Boolean, initMengerResolutionIndex: Int) {
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
                        val expanded = remember { mutableStateOf(false) }
                        val selectedIndex = remember { mutableStateOf(initMengerResolutionIndex) }
                        Row(horizontalArrangement = Arrangement.SpaceBetween,
                            modifier = Modifier.clickable {
                                expanded.value = !expanded.value
                            }) {
                            ListItemText(text = "Menger Sponge Resolution", textAlign = TextAlign.Start)
                            ListItemText(text = "(${InfoViewModel.mengerPrisonResolutions[selectedIndex.value]})", textAlign = TextAlign.Start)
                        }
                        DropdownMenu(expanded = expanded.value, onDismissRequest = { expanded.value = !expanded.value }) {
                            InfoViewModel.mengerPrisonResolutions.forEachIndexed { index, s ->
                                DropdownMenuItem(onClick = {
                                    selectedIndex.value = index
                                    expanded.value = false
                                    viewModel.onMengerPrisonResolutionSelected(index)
                                }) {
                                    val selectedEmoji = if (index == selectedIndex.value) "  🎞" else ""
                                    Text(text = s + selectedEmoji)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}