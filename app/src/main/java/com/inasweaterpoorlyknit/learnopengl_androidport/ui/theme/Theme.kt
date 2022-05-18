package com.inasweaterpoorlyknit.learnopengl_androidport.ui.theme

import androidx.compose.material.MaterialTheme
import androidx.compose.material.darkColors
import androidx.compose.material.lightColors
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.lifecycle.LifecycleOwner
import com.inasweaterpoorlyknit.learnopengl_androidport.OpenGLScenesApplication

private val LightColorPalette = lightColors(
    primary = Blue100,
    onPrimary = Color.Black,
    background = Grayscale4,
    surface = Grayscale5,
    /* Other default colors to override
    primaryVariant = RosyBrown2,
    secondary = ,
    secondaryVariant = ,
    onSecondary = ,
    onBackground = Color.Black,
    onSurface = Color.Black,
    */
)

private val DarkColorPalette = darkColors(
    primary = Blue100_dark,
    onPrimary = Color.Black,
    background = Grayscale1,
    surface = Grayscale0,
)

@Composable
fun OpenGLScenesTheme(
    lifeCycleOwner: LifecycleOwner,
    content: @Composable () -> Unit
) {
    val darkMode = remember { mutableStateOf(true) }
    (LocalContext.current.applicationContext as OpenGLScenesApplication).darkMode.observe(lifeCycleOwner) {
        darkMode.value = it
    }

    MaterialTheme(
        colors = if (darkMode.value) DarkColorPalette else LightColorPalette,
        typography = Typography,
        shapes = Shapes,
        content = content
    )
}