package org.kgajjar.mobileai.navigation

import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Search
import androidx.compose.material.icons.filled.Person
import androidx.compose.material.icons.filled.MonitorHeart
import androidx.compose.ui.graphics.vector.ImageVector

sealed class NavigationItem(
    val title: String,
    val icon: ImageVector
) {
    data object Home : NavigationItem(
        title = "Home",
        icon = Icons.Default.Home
    )

    data object Ultrasound : NavigationItem(
        title = "Ultrasound",
        icon = Icons.Default.MonitorHeart
    )

    data object Search : NavigationItem(
        title = "Search",
        icon = Icons.Default.Search
    )

    data object Profile : NavigationItem(
        title = "Profile",
        icon = Icons.Default.Person
    )
}

fun getAllNavigationItems(): List<NavigationItem> {
    return listOf(
        NavigationItem.Home,
        NavigationItem.Ultrasound,
        NavigationItem.Search,
        NavigationItem.Profile
    )
}
