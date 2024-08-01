package com.inasweaterpoorlyknit.scenes.ui

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material.icons.rounded.Code
import androidx.compose.material.icons.rounded.Reviews
import androidx.compose.material.icons.rounded.Web
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ElevatedButton
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.ComposeView
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalUriHandler
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.res.vectorResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.airbnb.mvrx.compose.collectAsState
import com.airbnb.mvrx.compose.mavericksViewModel
import com.inasweaterpoorlyknit.scenes.R
import com.inasweaterpoorlyknit.scenes.common.WebUrls
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MandelbrotScene
import com.inasweaterpoorlyknit.scenes.graphics.scenes.MengerPrisonScene
import com.inasweaterpoorlyknit.scenes.ui.theme.OpenGLScenesTheme
import com.inasweaterpoorlyknit.scenes.viewmodels.SettingsState
import com.inasweaterpoorlyknit.scenes.viewmodels.SettingsViewModel
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.launch

@AndroidEntryPoint
class SettingsFragment : Fragment() {

    private val headerModifier = Modifier.fillMaxWidth()
    private val itemHorizontalPadding = 8.dp
    private val itemModifier = Modifier.padding(horizontal = itemHorizontalPadding)
    private val dividerHorizontalPadding = itemHorizontalPadding * 2
    private val dividerModifier = Modifier.padding(horizontal = dividerHorizontalPadding, vertical = 8.dp)
    private val dividerThickness = 2.dp
    @Composable private fun settingsFontSize() = MaterialTheme.typography.bodyLarge.fontSize
    @Composable private fun settingsTitleFontSize() = MaterialTheme.typography.titleMedium.fontSize
    @Composable fun merlinsbag() = ImageVector.vectorResource(R.drawable.merlinsbag) // TODO: use correct icon

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        return ComposeView(requireContext()).apply {
            setContent {
                val settingsViewModel: SettingsViewModel = mavericksViewModel()
                val mengerResolutionIndex by settingsViewModel.collectAsState(SettingsState::mengerResolutionIndex)
                var mengerRowExpanded by remember { mutableStateOf(false) }
                val mandelbrotColorIndex by settingsViewModel.collectAsState(SettingsState::mandelbrotColorIndex)
                var mandelbrotRowExpanded by remember { mutableStateOf(false) }
                val uriHandler = LocalUriHandler.current

                SettingsScreen(
                    mengerResolutionIndex = mengerResolutionIndex,
                    mengerRowExpanded = mengerRowExpanded,
                    mandelbrotColorIndex = mandelbrotColorIndex,
                    mandelbrotRowExpanded = mandelbrotRowExpanded,
                    onClickDeveloper = { uriHandler.openUri(WebUrls.AUTHOR_WEBSITE) },
                    onClickSource = { uriHandler.openUri(WebUrls.SOURCE_CODE_URL) },
                    onClickMerlinsbag = { uriHandler.openUri(WebUrls.MERLINSBAG_URL) },
                    onClickRateAndReview = {
                        rateAndReviewRequest(
                            context = context,
                            onCompleted = { context.toast(R.string.thank_you) },
                            onPreviouslyCompleted = { uriHandler.openUri(WebUrls.ECCOHEDRA_URL) },
                            onError = { context.toast(R.string.try_again_later) },
                        )
                    },
                    onClickMandelbrotColor = {
                        mandelbrotRowExpanded = !mandelbrotRowExpanded
                    },
                    onSelectMandelbrotColor = {
                        mandelbrotRowExpanded = false
                        lifecycleScope.launch { settingsViewModel.onMandelbrotColorSelected(it) }
                    },
                    onClickMengerPrisonResolution = {
                        mengerRowExpanded = !mengerRowExpanded
                    },
                    onSelectMengerPrisonResolution = {
                        mengerRowExpanded = false
                        lifecycleScope.launch { settingsViewModel.onMengerPrisonResolutionSelected(it) }
                    },
                )
            }
        }
    }

    @Composable
    fun SettingsTitle(text: String) {
        Text(
            text = text,
            fontSize = settingsTitleFontSize(),
            color = MaterialTheme.colorScheme.onBackground,
            textAlign = TextAlign.Center,
            modifier = headerModifier,
        )
    }

    @Composable
    fun DropdownSettingsRow(
        title: String,
        indicator: @Composable (() -> Unit),
        expanded: Boolean,
        onClick: () -> Unit,
        onSelect: (Int) -> Unit,
        onDismiss: () -> Unit,
        items: List<String>,
        enabled: Boolean = true,
    ) {
        Column(
            horizontalAlignment = Alignment.End,
            modifier = itemModifier.fillMaxWidth(),
        ) {
            SettingsTextIndicatorButton(
                enabled = enabled,
                text = title,
                indicator = indicator,
                onClick = onClick,
            )
            Box {
                DropdownMenu(
                    expanded = expanded,
                    onDismissRequest = onDismiss,
                ) {
                    items.forEachIndexed { index, item ->
                        DropdownMenuItem(
                            text = { Text(text = item) },
                            onClick = { onSelect(index) },
                        )
                    }
                }
            }
        }
    }

    @Composable
    fun SettingsTextIndicatorButton(
        text: String,
        onClick: () -> Unit,
        modifier: Modifier = Modifier,
        indicator: @Composable (() -> Unit)? = null,
        enabled: Boolean = true,
    ) {
        ElevatedButton(
            onClick = onClick,
            enabled = enabled,
            shape = MaterialTheme.shapes.medium,
            modifier = modifier.fillMaxWidth()
        ) {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween,
                modifier = Modifier.fillMaxWidth(),
            ) {
                Text(
                    text = text,
                    fontSize = settingsFontSize(),
                )
                Spacer(modifier.width(itemHorizontalPadding))
                indicator?.invoke()
            }
        }
    }

    @Composable
    fun DeveloperRow(onClick: () -> Unit) = SettingsTextIndicatorButton(
        text = stringResource(R.string.developer),
        indicator = { Icon(ScenesIcons.Web, stringResource(R.string.developer)) },
        onClick = onClick,
        modifier = itemModifier,
    )

    @Composable
    fun SourceRow(onClick: () -> Unit) = SettingsTextIndicatorButton(
        text = stringResource(R.string.source),
        indicator = { Icon(ScenesIcons.Code, stringResource(R.string.source)) },
        onClick = onClick,
        modifier = itemModifier,
    )

    @Composable
    fun RateAndReviewRow(onClick: () -> Unit) = SettingsTextIndicatorButton(
        text = stringResource(R.string.rate_and_review),
        indicator = { Icon(ScenesIcons.Reviews, stringResource(R.string.rate_and_review)) },
        onClick = onClick,
        modifier = itemModifier,
    )

    @Composable
    fun MerlinsbagRow(onClick: () -> Unit) = SettingsTextIndicatorButton(
        text = stringResource(R.string.merlinsbag),
        indicator = { Icon(merlinsbag(), stringResource(R.string.merlinsbag), modifier = Modifier.size(24.dp)) },
        onClick = onClick,
        modifier = itemModifier,
    )

    @Composable
    fun versionName(): String {
        val context = LocalContext.current
        try {
            return context.packageManager.getPackageInfo(context.packageName, 0).versionName
        } catch(e: Exception){
            e.printStackTrace()
            return "?"
        }
    }

    @Composable
    fun VersionRow() {
        val textColor = MaterialTheme.colorScheme.onBackground
        Row(horizontalArrangement = Arrangement.SpaceBetween, modifier = Modifier.fillMaxWidth().padding(horizontal = 32.dp, vertical = 4.dp)){
            Text(
                text = stringResource(R.string.version),
                color = textColor,
            )
            Text(
                text = versionName(),
                color = textColor,
            )
        }
    }

    @Composable
    fun MandelbrotColorRow(
        selectedIndex: Int,
        expandedMenu: Boolean,
        onClick: () -> Unit,
        onSelectMandelbrotColor: (Int) -> Unit,
        onDismiss: () -> Unit,
    ) {
        val options = SettingsState.mandelbrotColors
        DropdownSettingsRow(
            title = stringResource(R.string.mandelbrot_color),
            indicator = {
                Text(
                    text = options[selectedIndex],
                    fontSize = settingsFontSize(),
                    textAlign = TextAlign.Right,
                    modifier = Modifier.fillMaxHeight()
                )
            },
            expanded = expandedMenu,
            items = options,
            onClick = onClick,
            onSelect = onSelectMandelbrotColor,
            onDismiss = onDismiss,
        )
    }

    @Composable
    fun MengerPrisonResolutionRow(
        selectedIndex: Int,
        expandedMenu: Boolean,
        onClick: () -> Unit,
        onSelectMengerPrisonResolution: (Int) -> Unit,
        onDismiss: () -> Unit,
    ) {
        val options = SettingsState.mengerResolutionStrings
        DropdownSettingsRow(
            title = stringResource(R.string.menger_resolution),
            indicator = {
                Text(
                    text = options[selectedIndex],
                    fontSize = settingsFontSize(),
                    textAlign = TextAlign.Right,
                    modifier = Modifier.fillMaxHeight()
                )
            },
            expanded = expandedMenu,
            items = options,
            onClick = onClick,
            onSelect = onSelectMengerPrisonResolution,
            onDismiss = onDismiss,
        )
    }


    @Composable
    fun SettingsScreen(
        mengerResolutionIndex: Int,
        mengerRowExpanded: Boolean,
        mandelbrotColorIndex: Int,
        mandelbrotRowExpanded: Boolean,
        onClickDeveloper: () -> Unit,
        onClickSource: () -> Unit,
        onClickMerlinsbag: () -> Unit,
        onClickRateAndReview: () -> Unit,
        onClickMandelbrotColor: () -> Unit,
        onSelectMandelbrotColor: (Int) -> Unit,
        onClickMengerPrisonResolution: () -> Unit,
        onSelectMengerPrisonResolution: (Int) -> Unit
    ){
        val rows = staggeredHorizontallyAnimatedComposables(
            content = listOf(
                { SettingsTitle(stringResource(R.string.settings)) },
                {
                    MandelbrotColorRow(
                        selectedIndex = mandelbrotColorIndex,
                        expandedMenu = mandelbrotRowExpanded,
                        onClick = onClickMandelbrotColor,
                        onSelectMandelbrotColor = onSelectMandelbrotColor,
                        onDismiss = onClickMandelbrotColor,
                    )
                },
                {
                    MengerPrisonResolutionRow(
                        selectedIndex = mengerResolutionIndex,
                        expandedMenu = mengerRowExpanded,
                        onClick = onClickMengerPrisonResolution,
                        onSelectMengerPrisonResolution = onSelectMengerPrisonResolution,
                        onDismiss = onClickMengerPrisonResolution,
                    )
                },
                { HorizontalDivider(thickness = dividerThickness, modifier = dividerModifier) },
                { SettingsTitle(stringResource(R.string.about)) },
                { DeveloperRow(onClickDeveloper) },
                { SourceRow(onClickSource) },
                { VersionRow() },
                { HorizontalDivider(thickness = dividerThickness, modifier = dividerModifier) },
                { SettingsTitle(stringResource(R.string.etc)) },
                { RateAndReviewRow(onClickRateAndReview) },
                { MerlinsbagRow(onClickMerlinsbag) },
            )
        )
        OpenGLScenesTheme {
            LazyColumn(
                contentPadding = PaddingValues(vertical = halfListPadding),
                modifier = Modifier
                    .background(color = MaterialTheme.colorScheme.background)
                    .fillMaxSize()
            ) {
                item { Spacer(modifier = Modifier.height(8.dp)) }
                items(rows.size) { index -> rows[index]() }
                item { Spacer(modifier = Modifier.height(8.dp)) }
            }
        }
    }

    @Preview
    @Composable
    fun SettingsListPreview() {
        SettingsScreen(
            mengerResolutionIndex = MengerPrisonScene.DEFAULT_RESOLUTION_INDEX,
            mengerRowExpanded = false,
            mandelbrotColorIndex = MandelbrotScene.DEFAULT_COLOR_INDEX,
            mandelbrotRowExpanded = false,
            onClickSource = {},
            onClickDeveloper = {},
            onClickMerlinsbag = {},
            onClickMandelbrotColor = {},
            onClickMengerPrisonResolution = {},
            onClickRateAndReview = {},
            onSelectMengerPrisonResolution = {},
            onSelectMandelbrotColor = {},
        )
    }
}

