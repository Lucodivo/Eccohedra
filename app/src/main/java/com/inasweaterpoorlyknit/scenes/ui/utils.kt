package com.inasweaterpoorlyknit.scenes.ui

import android.app.Activity
import android.content.Context
import android.content.ContextWrapper
import android.content.Intent
import android.graphics.Color
import android.net.Uri
import android.os.Build
import android.util.Log
import android.view.View
import androidx.activity.ComponentActivity
import androidx.annotation.StringRes
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat
import androidx.fragment.app.Fragment
import com.google.android.play.core.review.ReviewException
import com.google.android.play.core.review.ReviewManagerFactory

fun Context.toast(msg: String) = android.widget.Toast.makeText(this, msg, android.widget.Toast.LENGTH_SHORT).show()
fun Context.toast(@StringRes msg: Int) = android.widget.Toast.makeText(this, resources.getString(msg), android.widget.Toast.LENGTH_SHORT).show()

fun Context.getActivity(): ComponentActivity? = when(this) {
    is ComponentActivity -> this
    is ContextWrapper -> baseContext.getActivity()
    else -> null
}

fun Activity.openWebPage(url: String) {
    val webpage: Uri =
        Uri.parse(url)
    val intent = Intent(Intent.ACTION_VIEW, webpage)
    startActivity(intent)
}

fun Fragment.openWebPage(url: String) {
    val webpage: Uri =
        Uri.parse(url)
    val intent = Intent(Intent.ACTION_VIEW, webpage)
    startActivity(intent)
}

fun Activity.showSystemUI() {
    if(Build.VERSION.SDK_INT >= 30) {
        window.apply {
            setDecorFitsSystemWindows(true)
            statusBarColor = Color.BLACK
        }
        WindowInsetsControllerCompat(window, window.decorView).show(WindowInsetsCompat.Type.navigationBars())
    } else { // TODO: Test if this even works
        window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_VISIBLE
    }
}

fun Activity.hideSystemUI() {
    if(Build.VERSION.SDK_INT >= 30) {
        window.apply {
            setDecorFitsSystemWindows(false) // fill window
            statusBarColor = Color.TRANSPARENT // set
        }

        WindowInsetsControllerCompat(window, window.decorView).let { controller ->
            // hide navigation buttons
            controller.hide(WindowInsetsCompat.Type.navigationBars())
            // allow navbar to show up after swipe
            controller.systemBarsBehavior = WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
        }
    } else { // TODO: System visibility is deprecated, remove when minSDK is 30+
        window.decorView.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or // hide the navigation
                        View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or // lay out view as if the navigation will be hidden
                        View.SYSTEM_UI_FLAG_IMMERSIVE or // used with HIDE_NAVIGATION to remain interactive when hiding navigation
                        View.SYSTEM_UI_FLAG_FULLSCREEN or // fullscreen
                        View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or // lay out view as if fullscreen
                        View.SYSTEM_UI_FLAG_LAYOUT_STABLE) // stable view of content (layout view size doesn't change)
    }
}

fun rateAndReviewRequest(
    context: Context,
    onPreviouslyCompleted: () -> Unit,
    onCompleted: () -> Unit,
    onError: () -> Unit,
) {
    val manager = ReviewManagerFactory.create(context)
    val request = manager.requestReviewFlow()
    request.addOnCompleteListener { task ->
        if(task.isSuccessful) {
            val reviewInfo = task.result
            val startNanoTime = System.nanoTime()
            val flow = manager.launchReviewFlow(context.getActivity()!!, reviewInfo)
            flow.addOnCompleteListener { _ ->
                if(System.nanoTime() - startNanoTime < 200_000_000) {
                    // Assume user has already reviewed and send them to the app store
                    onPreviouslyCompleted()
                } else {
                    // Assume user has potentially attempted to review and thank them
                    onCompleted()
                }
            }
        } else {
            val reviewException = task.exception as ReviewException
            Log.e("SettingsScreen", "Error requesting review: ${reviewException.message}")
            onError()
        }
    }
}