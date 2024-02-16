plugins {
    id("com.android.library")
}

android {
    namespace = "com.inasweaterpoorlyknit.native_scenes"
    compileSdk = 33

    defaultConfig {
        minSdk = 24
        consumerProguardFiles("consumer-rules.pro")
        externalNativeBuild {
            cmake {
                cppFlags("-std=c++17")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path("CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildTypes {
        debug {
            ndk {
                abiFilters += listOf("arm64-v8a")
            }
        }
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
}