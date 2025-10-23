package org.kgajjar.mobileai

import androidx.compose.ui.window.Window
import androidx.compose.ui.window.application

fun main() = application {
    Window(
        onCloseRequest = ::exitApplication,
        title = "MobileAI",
    ) {
        App()
    }
}