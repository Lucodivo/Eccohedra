package com.inasweaterpoorlyknit.scenes.ui

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.AnimatedVisibilityScope
import androidx.compose.animation.core.Animatable
import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.TweenSpec
import androidx.compose.animation.slideInHorizontally
import androidx.compose.animation.slideOutHorizontally
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.wrapContentHeight
import androidx.compose.material.icons.Icons
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Shape
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

val listItemFontSize = 20.sp
val listPadding = 8.dp
val halfListPadding = listPadding / 2
val listItemTextPadding = 10.dp

val ScenesIcons = Icons.Rounded

@Composable
fun ScenesListItem(
    modifier: Modifier = Modifier,
    shape: Shape = MaterialTheme.shapes.large,
    content: @Composable () -> Unit
) {
    Surface(
        tonalElevation = 1.dp,
        shadowElevation = 1.dp,
        shape = shape,
        content = content,
        modifier = modifier
            .padding(horizontal = listPadding, vertical = halfListPadding)
            .fillMaxWidth()
    )
}

@Composable
fun ListItemText(
    modifier: Modifier = Modifier,
    text: String,
    textAlign: TextAlign = TextAlign.Center,
    color: Color = Color.Unspecified
) {
    Text(
        text = text,
        fontSize = listItemFontSize,
        textAlign = textAlign,
        color = color,
        modifier = modifier
            .wrapContentHeight(Alignment.CenterVertically) // center vertically
            .padding(all = listItemTextPadding)
    )
}

@Composable
fun ListItemTextWithLeftIcon(
    modifier: Modifier = Modifier,
    text: String,
    textColor: Color = Color.Unspecified,
    icon: ImageVector
) {
    Row(horizontalArrangement = Arrangement.Center, modifier = modifier) {
        Icon(icon,
            contentDescription = icon.name,
            modifier = Modifier
                .align(alignment = Alignment.CenterVertically))
        ListItemText(text = text, color = textColor)
    }
}

@Composable
fun ListItemTextWithRightIcon(
    modifier: Modifier = Modifier,
    text: String,
    textColor: Color = Color.Unspecified,
    icon: ImageVector
) {
    Row(horizontalArrangement = Arrangement.Center, modifier = modifier) {
        ListItemText(text = text, color = textColor)
        Icon(icon,
            contentDescription = icon.name,
            modifier = Modifier
                .align(alignment = Alignment.CenterVertically))
    }
}

@Composable
fun ListItemSwitch(text: String, defaultState: Boolean, onClick: (Boolean) -> Unit) {
    val checkedState = remember { mutableStateOf(defaultState) }
    Row(horizontalArrangement = Arrangement.SpaceBetween,
        modifier = Modifier.clickable {
            val newState = !checkedState.value
            checkedState.value = newState
            onClick(newState)
        }) {
        Text(
            text = text,
            fontSize = listItemFontSize,
            textAlign = TextAlign.Center,
            modifier = Modifier
                .align(alignment = Alignment.CenterVertically)
                .padding(horizontal = listItemTextPadding)
        )
        Switch(checked = checkedState.value,
            modifier = Modifier
                .align(alignment = Alignment.CenterVertically)
                .padding(horizontal = listItemTextPadding),
            onCheckedChange = {
                val newState = !checkedState.value
                checkedState.value = newState
                onClick(newState)
            })
    }
}

@Composable
fun ListItemDropdown(titleText: String, items: List<String>, selectedIndex: Int, selectedDecorationText: String = "", onItemClicked: (index: Int) -> Unit) {
    val expanded = remember { mutableStateOf(false) }
    Row(horizontalArrangement = Arrangement.SpaceBetween,
        modifier = Modifier.clickable {
            expanded.value = !expanded.value
        }) {
        ListItemText(text = titleText, textAlign = TextAlign.Start)
        ListItemText(text = "(${items[selectedIndex]})", textAlign = TextAlign.Start)
    }
    DropdownMenu(expanded = expanded.value, onDismissRequest = { expanded.value = !expanded.value }) {
        items.forEachIndexed { index, s ->
            val selectedDecoration = if (index == selectedIndex) "  $selectedDecorationText" else ""
            DropdownMenuItem(
                text = { Text(text = s + selectedDecoration) },
                onClick = {
                onItemClicked(index)
                expanded.value = false
            })
        }
    }
}

@Composable
fun staggeredHorizontallyAnimatedComposables(
    content: List<@Composable AnimatedVisibilityScope.() -> Unit>,
    millisecondsPerRow: Int = 30,
): List<@Composable () -> Unit> {
    val animationFloat = remember { Animatable(initialValue = 0.0f) }
    LaunchedEffect(content.size) {
        animationFloat.animateTo(
            targetValue = content.size * 0.1f + 0.1f, // +0.1 as a safety buffer
            animationSpec = TweenSpec(
                durationMillis = millisecondsPerRow * content.size,
                easing = LinearEasing,
            )
        )
    }
    return content.mapIndexed { index, item -> {
        AnimatedVisibility(
            visible = animationFloat.value >= (0.1f * (index + 1)),
            enter = slideInHorizontally(initialOffsetX = { fullWidth -> fullWidth }),
            exit = slideOutHorizontally(targetOffsetX = { fullWidth -> fullWidth }),
            content = item,
        )
    }}
}
