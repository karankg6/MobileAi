package org.kgajjar.mobileai

interface Platform {
    val name: String
}

expect fun getPlatform(): Platform