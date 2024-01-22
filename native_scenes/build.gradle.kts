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
            // NOTE: this can make builds faster but absolutely cannot be present when creating a signed bundle
            //  destined for the Play Store.
            ndk {
                abiFilters += listOf("arm64-v8a")
            }
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
}