plugins {
    id("com.android.application")
    id("kotlin-android")

    id("com.google.gms.google-services")
    id("com.google.firebase.crashlytics")

    id("androidx.navigation.safeargs.kotlin")

    id("kotlin-kapt")
    id("com.google.dagger.hilt.android")

    id("com.google.protobuf").version("0.9.4")
}

android {
    compileSdk = 34
    ndkVersion = "25.2.9519653"
    namespace = "com.inasweaterpoorlyknit.scenes"

    defaultConfig {
        applicationId = "com.inasweaterpoorlyknit.learnopengl_androidport"
        minSdk = 24
        targetSdk = 33
        versionCode = 18
        versionName = "1.2.1"
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        signingConfig = signingConfigs.getByName("debug")

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

    implementation("com.github.lucodivo:NoopMathKt:v0.2.4-alpha")
    implementation(project(":native_scenes"))

    // Kotlin
    implementation("org.jetbrains.kotlin:kotlin-stdlib-jdk7:${kotlinCompilerVersion}")

    // Google
    implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("com.google.android.material:material:1.11.0")
    implementation("androidx.legacy:legacy-support-v4:1.0.0")
    implementation("androidx.constraintlayout:constraintlayout:2.1.4")
    implementation("androidx.preference:preference-ktx:1.2.1")
    implementation("androidx.fragment:fragment-ktx:1.6.1")

    // Navigation
    val navVersion = "2.7.7"
    implementation("androidx.navigation:navigation-fragment-ktx:$navVersion")
    implementation("androidx.navigation:navigation-ui-ktx:$navVersion")
    implementation("androidx.navigation:navigation-fragment-ktx:$navVersion")
    implementation("androidx.navigation:navigation-ui-ktx:$navVersion")
    implementation("androidx.navigation:navigation-dynamic-features-fragment:$navVersion")
    androidTestImplementation("androidx.navigation:navigation-testing:$navVersion")
    implementation("androidx.navigation:navigation-compose:$navVersion")

    // Compose
    implementation("androidx.activity:activity-compose:1.8.2") // Integration with activities
    implementation("androidx.compose.material3:material3:1.2.0")
    implementation("androidx.compose.material:material-icons-extended:1.6.0")
    implementation("androidx.compose.animation:animation:1.6.0") // Animations
    implementation("androidx.compose.ui:ui-tooling:1.6.0") // Tooling support (Previews, etc.)
    implementation("androidx.lifecycle:lifecycle-viewmodel-compose:2.7.0") // Integration with ViewModel
    //androidTestImplementation("androidx.compose.ui:ui-test-junit4:1.1.1") // UI Tests

    // MvRx
    val mvrxVersion = "3.0.9"
    implementation("com.airbnb.android:mavericks:$mvrxVersion")
    implementation("com.airbnb.android:mavericks-hilt:$mvrxVersion")
    implementation("com.airbnb.android:mavericks-compose:$mvrxVersion")

    // analytics
    implementation(platform("com.google.firebase:firebase-bom:32.2.3"))
    implementation("com.google.firebase:firebase-crashlytics-ktx")
    implementation("com.google.firebase:firebase-analytics-ktx")

    // testing
    testImplementation("junit:junit:4.13.2")
    androidTestImplementation("androidx.test.ext:junit:1.1.5")
    androidTestImplementation("androidx.test.espresso:espresso-core:3.5.1")

    // Hilt (Dependency Injection)
    implementation("com.google.dagger:hilt-android:2.50")
    kapt("com.google.dagger:hilt-compiler:2.50")
    androidTestImplementation("com.google.dagger:hilt-android-testing:2.50")
    kaptAndroidTest("com.google.dagger:hilt-compiler:2.50")
    testImplementation("com.google.dagger:hilt-android-testing:2.50")
    kaptTest("com.google.dagger:hilt-compiler:2.50")

    // Proto DataStore
    implementation("androidx.datastore:datastore:1.0.0")
//    implementation("androidx.datastore:datastore-rxjava2:1.0.0")
//    implementation("androidx.datastore:datastore-rxjava3:1.0.0")
// Starting from Protobuf 3.8.0, use the lite runtime library
    implementation("com.google.protobuf:protobuf-kotlin-lite:3.24.4")
}

protobuf {
    protoc {
        artifact = "com.google.protobuf:protoc:3.24.4"
    }
    generateProtoTasks {
        all().forEach { task ->
            task.builtins {
                register("java") {
                    option("lite")
                }
                register("kotlin") {
                    option("lite")
                }
            }
        }
    }
}

// Allow references to generated code
kapt {
    correctErrorTypes = true
}