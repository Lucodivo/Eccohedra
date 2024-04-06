// Top-level build file where you can add configuration options common to all sub-projects/modules.
buildscript {
    // Kotln and compose compiler must be compatible
    // check the following link for more details
    // https://developer.android.com/jetpack/androidx/releases/compose-kotlin
    extra.set("kotlinCompilerVersion", "1.9.10")
    extra.set("composeCompilerVersion", "1.5.3")

    repositories { // used for project dependencies below
        google()
        mavenCentral()
    }

    dependencies {
        val kotlinCompilerVersion: String by rootProject.extra
        val navVersion = "2.7.7"

        classpath("com.android.tools.build:gradle:8.3.1")
        classpath("org.jetbrains.kotlin:kotlin-gradle-plugin:${kotlinCompilerVersion}")

        classpath("com.google.gms:google-services:4.3.15")
        classpath("com.google.firebase:firebase-crashlytics-gradle:2.9.9")

        classpath("androidx.navigation:navigation-safe-args-gradle-plugin:$navVersion")

        classpath("com.google.dagger:hilt-android-gradle-plugin:2.50")
    }
}

plugins {
    id("com.google.dagger.hilt.android") version "2.44" apply false
}

allprojects { // used for dependencies in module gradles
    repositories {
        google()
        mavenCentral()
        maven("https://jitpack.io")
    }
}

tasks.register<Delete>("clean").configure {
    delete(rootProject.buildDir)
}