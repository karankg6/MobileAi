package org.kgajjar.mobileai

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import org.jetbrains.compose.ui.tooling.preview.Preview
import org.kgajjar.mobileai.navigation.NavigationItem
import org.kgajjar.mobileai.navigation.getAllNavigationItems
import org.kgajjar.mobileai.screens.HomeScreen
import org.kgajjar.mobileai.screens.SearchScreen
import org.kgajjar.mobileai.screens.ProfileScreen
import org.kgajjar.mobileai.screens.UltrasoundScreen
import org.kgajjar.mobileai.theme.DarkColorScheme

@Composable
@Preview
fun App() {
    MaterialTheme(
        colorScheme = DarkColorScheme
    ) {
        var selectedItem by remember { mutableStateOf<NavigationItem>(NavigationItem.Home) }
        val navigationItems = getAllNavigationItems()

        Scaffold(
            bottomBar = {
                NavigationBar(
                    containerColor = MaterialTheme.colorScheme.surface,
                    contentColor = MaterialTheme.colorScheme.onSurface
                ) {
                    navigationItems.forEach { item ->
                        NavigationBarItem(
                            icon = {
                                Icon(
                                    imageVector = item.icon,
                                    contentDescription = item.title
                                )
                            },
                            label = { Text(item.title) },
                            selected = selectedItem == item,
                            onClick = { selectedItem = item },
                            colors = NavigationBarItemDefaults.colors(
                                selectedIconColor = MaterialTheme.colorScheme.primary,
                                selectedTextColor = MaterialTheme.colorScheme.primary,
                                unselectedIconColor = MaterialTheme.colorScheme.onSurface.copy(alpha = 0.6f),
                                unselectedTextColor = MaterialTheme.colorScheme.onSurface.copy(alpha = 0.6f),
                                indicatorColor = MaterialTheme.colorScheme.primaryContainer
                            )
                        )
                    }
                }
            }
        ) { paddingValues ->
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(paddingValues)
            ) {
                when (selectedItem) {
                    NavigationItem.Home -> HomeScreen()
                    NavigationItem.Ultrasound -> UltrasoundScreen()
                    NavigationItem.Search -> SearchScreen()
                    NavigationItem.Profile -> ProfileScreen()
                }
            }
        }
    }
}