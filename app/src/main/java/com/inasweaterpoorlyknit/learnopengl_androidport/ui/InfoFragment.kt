package com.inasweaterpoorlyknit.learnopengl_androidport.ui

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
import androidx.compose.material.icons.rounded.OpenInBrowser
import androidx.compose.material.icons.rounded.Web
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.fragment.app.Fragment
import androidx.fragment.app.activityViewModels
import com.inasweaterpoorlyknit.learnopengl_androidport.OpenGLScenesApplication
import com.inasweaterpoorlyknit.learnopengl_androidport.R
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MandelbrotScene
import com.inasweaterpoorlyknit.learnopengl_androidport.graphics.scenes.MengerPrisonScene
import com.inasweaterpoorlyknit.learnopengl_androidport.ui.theme.OpenGLScenesTheme
import com.inasweaterpoorlyknit.learnopengl_androidport.viewmodels.InfoViewModel

class InfoFragment : Fragment() {
    private val activityViewModel: InfoViewModel by activityViewModels()

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        activityViewModel.webRequest.observe(viewLifecycleOwner) { openWebPage(it) }
        return ComposeView(requireContext()).apply {
            setContent {
                val darkMode = remember { mutableStateOf(true) }
                val app = LocalContext.current.applicationContext
                if(app is OpenGLScenesApplication) { // this check is only necessary for compose preview
                    app.darkMode.observe(viewLifecycleOwner) {
                        darkMode.value = it
                    }
                }
                InfoList(
                    mengerResolutionIndex = activityViewModel.getMengerSpongeResolutionIndex(),
                    mandelbrotColorIndex = activityViewModel.getMandelbrotColorIndex(),
                    onContactPress = { activityViewModel.onContactPress() },
                    onSourcePress = { activityViewModel.onSourcePress() },
                    onMandelbrotColorSelect = { activityViewModel.onMandelbrotColorSelected(it) },
                    onMengerPrisonResolutionSelect = { activityViewModel.onMengerPrisonResolutionSelected(it) })
            }
        }
    }
}

@Preview
@Composable
fun InfoListPreview() {
    InfoList(MengerPrisonScene.defaultResolutionIndex, MandelbrotScene.defaultColorIndex)
}

@Composable
fun InfoList(mengerResolutionIndex: Int = MengerPrisonScene.defaultResolutionIndex,
             mandelbrotColorIndex: Int = MandelbrotScene.defaultColorIndex,
             onContactPress: () -> Unit = {},
             onSourcePress: () -> Unit = {},
             onMandelbrotColorSelect: (Int) -> Unit = {},
             onMengerPrisonResolutionSelect: (Int) -> Unit = {}){
    OpenGLScenesTheme() {
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

            // Scenes Title
            item {
                ScenesListItem {
                    ListItemText(text = stringResource(R.string.scenes), color = MaterialTheme.colorScheme.onPrimary, modifier = Modifier.background(color = MaterialTheme.colorScheme.primary))
                }
            }

            // Author Contact
            item {
                ScenesListItem {
                    ListItemTextWithRightIcon(
                        "Connor Alexander Haskins", icon = ScenesIcons.Web,
                        modifier = Modifier.clickable { onContactPress() })
                }
            }

            // Source Code
            item {
                ScenesListItem {
                    ListItemTextWithRightIcon(text = stringResource(R.string.source), icon = ScenesIcons.Code,
                        modifier = Modifier.clickable { onSourcePress() })
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
                        items = InfoViewModel.mengerPrisonResolutions,
                        initSelectedIndex = mengerResolutionIndex,
                        selectedDecorationText = "🎞"
                    ){ onMengerPrisonResolutionSelect(it) }
                }
            }

            // Mandelbrot Color
            item {
                ScenesListItem {
                    ListItemDropdown(
                        titleText = stringResource(R.string.mandelbrot_color),
                        items = InfoViewModel.mandelbrotColors,
                        initSelectedIndex = mandelbrotColorIndex,
                        selectedDecorationText = "🖌"
                    ){ onMandelbrotColorSelect(it) }
                }
            }
        }
    }
}
