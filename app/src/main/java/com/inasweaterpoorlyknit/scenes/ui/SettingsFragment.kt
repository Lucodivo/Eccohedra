package com.inasweaterpoorlyknit.scenes.ui

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material.icons.rounded.Code
import androidx.compose.material.icons.rounded.Web
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.airbnb.mvrx.compose.collectAsState
import com.airbnb.mvrx.compose.mavericksViewModel
import com.inasweaterpoorlyknit.scenes.R
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MandelbrotScene
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MengerPrisonScene
import com.inasweaterpoorlyknit.scenes.ui.theme.OpenGLScenesTheme
import com.inasweaterpoorlyknit.scenes.viewmodels.SettingsState
import com.inasweaterpoorlyknit.scenes.viewmodels.SettingsViewModel
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.launch

@AndroidEntryPoint
class SettingsFragment : Fragment() {

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        return ComposeView(requireContext()).apply {
            setContent {
                val settingsViewModel: SettingsViewModel = mavericksViewModel()
                val mengerResolutionIndex by settingsViewModel.collectAsState(SettingsState::mengerResolutionIndex)
                val mandelbrotColorIndex by settingsViewModel.collectAsState(SettingsState::mandelbrotColorIndex)

                SettingsList(
                    mengerResolutionIndex = mengerResolutionIndex,
                    mandelbrotColorIndex = mandelbrotColorIndex,
                    onContactPress = { openWebPage(SettingsState.WEBSITE_URL) },
                    onSourcePress = { openWebPage(SettingsState.SOURCE_URL) },
                    onMandelbrotColorSelect = { lifecycleScope.launch { settingsViewModel.onMandelbrotColorSelected(it) }},
                    onMengerPrisonResolutionSelect = { lifecycleScope.launch { settingsViewModel.onMengerPrisonResolutionSelected(it) }})
            }
        }
    }

    @Preview
    @Composable
    fun SettingsListPreview() {
        SettingsList(MengerPrisonScene.DEFAULT_RESOLUTION_INDEX, MandelbrotScene.DEFAULT_COLOR_INDEX)
    }

    @Composable
    fun SettingsList(mengerResolutionIndex: Int = MengerPrisonScene.DEFAULT_RESOLUTION_INDEX,
                     mandelbrotColorIndex: Int = MandelbrotScene.DEFAULT_COLOR_INDEX,
                     onContactPress: () -> Unit = {},
                     onSourcePress: () -> Unit = {},
                     onMandelbrotColorSelect: (Int) -> Unit = {},
                     onMengerPrisonResolutionSelect: (Int) -> Unit = {}){
        OpenGLScenesTheme {
            LazyColumn(
                contentPadding = PaddingValues(vertical = halfListPadding),
                modifier = Modifier
                    .background(color = MaterialTheme.colorScheme.background)
                    .fillMaxSize()
            ) {
                // About Header
                item {
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = stringResource(R.string.info),
                        color = MaterialTheme.colorScheme.onBackground,
                        fontSize = listItemFontSize,
                        textAlign = TextAlign.Center,
                        modifier = Modifier
                            .padding(all = listItemTextPadding)
                            .fillMaxWidth()
                    )
                }

                // Author Contact
                item {
                    ScenesListItem {
                        ListItemTextWithRightIcon(
                            modifier = Modifier.clickable { onContactPress() },
                            "Connor Alexander Haskins",
                            icon = ScenesIcons.Web
                        )
                    }
                }

                // Source Code
                item {
                    ScenesListItem {
                        ListItemTextWithRightIcon(
                            modifier = Modifier.clickable { onSourcePress() },
                            text = stringResource(R.string.source),
                            icon = ScenesIcons.Code
                        )
                    }
                }

                // Settings Header
                item {
                    Spacer(modifier = Modifier.height(40.dp))
                    Text(
                        text = stringResource(R.string.settings),
                        color = MaterialTheme.colorScheme.onBackground,
                        fontSize = listItemFontSize,
                        textAlign = TextAlign.Center,
                        modifier = Modifier
                            .padding(all = listItemTextPadding)
                            .fillMaxWidth()
                    )
                }

                // Menger Prison Resolution
                item {
                    ScenesListItem {
                        ListItemDropdown(
                            titleText = stringResource(R.string.menger_sponge_resolution),
                            items = SettingsState.mengerResolutionStrings,
                            selectedIndex = mengerResolutionIndex,
                            selectedDecorationText = "🎞"
                        ){ onMengerPrisonResolutionSelect(it) }
                    }
                }

                // Mandelbrot Color
                item {
                    ScenesListItem {
                        ListItemDropdown(
                            titleText = stringResource(R.string.mandelbrot_color),
                            items = SettingsState.mandelbrotColors,
                            selectedIndex = mandelbrotColorIndex,
                            selectedDecorationText = "🖌"
                        ){ onMandelbrotColorSelect(it) }
                    }
                }
            }
        }
    }
}

