plugins {
    id("com.android.application")
    id("kotlin-android")

    id("com.google.gms.google-services")
    id("com.google.firebase.crashlytics")

    // TODO: Hilt
//    id("kotlin-kapt")
//    id("dagger.hilt.android.plugin") - uncomment to bring Hilt back into project
}

android {
    compileSdk = 34
    ndkVersion = "25.2.9519653"

    defaultConfig {
        applicationId = "com.inasweaterpoorlyknit.learnopengl_androidport"
        minSdk = 24
        targetSdk = 33
        versionCode = 16
        versionName = "1.2.0"
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        compileOptions {
            sourceCompatibility(JavaVersion.VERSION_1_8)
            targetCompatibility(JavaVersion.VERSION_1_8)
        }

        kotlinOptions {
            jvmTarget = "1.8"
        }

        buildFeatures {
            // Enables Jetpack Compose for this module
            compose = true
            viewBinding = true
        }

        composeOptions {
            val composeCompilerVersion: String by rootProject.extra
            kotlinCompilerExtensionVersion = composeCompilerVersion
        }

        packagingOptions {
            resources {
                excludes += listOf(
                        "META-INF/DEPENDENCIES",
                        "META-INF/LICENSE",
                        "META-INF/LICENSE.txt",
                        "META-INF/license.txt",
                        "META-INF/NOTICE",
                        "META-INF/NOTICE.txt",
                        "META-INF/notice.txt",
                        "META-INF/ASL2.0",
                        "META-INF/build.kotlin_module"
                )
            }
        }

        namespace = "com.inasweaterpoorlyknit.learnopengl_androidport"
        signingConfig = signingConfigs.getByName("debug")
    }

    buildTypes {
        getByName("release") {
            // Enables code shrinking, obfuscation, and optimization
            isMinifyEnabled = true
            // Enables resource shrinking
            isShrinkResources = true
            // Includes the default ProGuard rules files that are packaged with the Android Gradle plugin
            setProguardFiles(listOf(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro"))
            resValue("bool", "FIREBASE_ANALYTICS_DEACTIVATED", "false")
        }
        getByName("debug") {
            isMinifyEnabled = false
            setProguardFiles(listOf(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro"))
            isDebuggable = true
            resValue("bool", "FIREBASE_ANALYTICS_DEACTIVATED", "true")
        }
    }

    androidResources {
        noCompress += listOf("modl", "cbtx", "tx", "vert", "frag")
    }
}

dependencies {
    val kotlinCompilerVersion: String by rootProject.extra

    implementation(project(":matrices"))
    implementation(project(":native_scenes"))

    // all binary .jar dependencies in libs folder listOf(NONE CURRENTLY)
//    implementation fileTree(mapOf("dir" to "libs", "include" to listOf("*.jar")))

    // Kotlin
    implementation("org.jetbrains.kotlin:kotlin-stdlib-jdk7:${kotlinCompilerVersion}")

    // Google
    implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("com.google.android.material:material:1.9.0")
    implementation("androidx.legacy:legacy-support-v4:1.0.0")
    implementation("androidx.constraintlayout:constraintlayout:2.1.4")
    implementation("androidx.preference:preference-ktx:1.2.1")
    implementation("androidx.fragment:fragment-ktx:1.6.1")

    // Compose
    implementation("androidx.activity:activity-compose:1.7.2") // Integration with activities
    implementation("androidx.compose.material:material:1.5.0") // Compose Material Design
    implementation("androidx.compose.material:material-icons-extended:1.5.0")
    implementation("androidx.compose.animation:animation:1.5.0") // Animations
    implementation("androidx.compose.ui:ui-tooling:1.5.0") // Tooling support (Previews, etc.)
    implementation("androidx.lifecycle:lifecycle-viewmodel-compose:2.6.1") // Integration with ViewModels
    //androidTestImplementation("androidx.compose.ui:ui-test-junit4:1.1.1") // UI Tests

    // analytics
    implementation(platform("com.google.firebase:firebase-bom:32.2.3"))
    implementation("com.google.firebase:firebase-crashlytics-ktx")
    implementation("com.google.firebase:firebase-analytics-ktx")

    // testing
    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test.ext:junit:1.1.5")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.5.1")

    // Hilt (Dependency Injection)
    // TODO: Hilt
//    implementation("com.google.dagger:hilt-android:2.38.1")
//    kapt("com.google.dagger:hilt-compiler:2.38.1")
}