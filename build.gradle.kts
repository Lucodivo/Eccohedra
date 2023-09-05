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

        classpath("com.android.tools.build:gradle:8.1.1")
        classpath("org.jetbrains.kotlin:kotlin-gradle-plugin:${kotlinCompilerVersion}")

        classpath("com.google.gms:google-services:4.3.15")
        classpath("com.google.firebase:firebase-crashlytics-gradle:2.9.9")

        // TODO: Hilt
        //classpath("com.google.dagger:hilt-android-gradle-plugin:2.40.1")
    }
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