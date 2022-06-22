#### When does our native code come into play?
- In the AndroidManifest we connect a given NativeActivity with it's native implementation via a specified C++ library.
- The entry point for our NativeActivity is the function in our specified C++ library with the signature `void android_main(android_app* app)`. 
- `android_main()` is defined and entered from `android_native_app_glue.c`.
- `android_main()` is called sometime after the OS spins up our NativeActivity

#### Acquiring AAssetManager
The single parameter of the `android_main()` entry point of the native activity is `android_app* app`. To access
the native AAssetManager pointer, simply use the following:
- `app->activity->assetManager`

[AAssetManager's available API is defined here.](https://developer.android.com/ndk/reference/group/asset) And
an AAssetManager may be shared across multiple threads.

#### General tips
- When acquiring classes, fields, or methods from Java, ensure you are on the application's main thread.
    - JNI functions are called from the main thread
    - 'android_main' is on a separate thread and NOT on the main thread. However, the `android_app` parameter can be used to attach to the main thread and might look something like this:
		- `androidApp->activity->vm->AttachCurrentThread(&env, nullptr)`
		- [More information about attaching threads to the JVM can be found here.](https://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/invocation.html)
    - [More information on things to look out for when finding Java classes/fields/methods from C++](https://developer.android.com/training/articles/perf-jni.html#faq:-why-didnt-findclass-find-my-class)

#### References
- [Native concepts overview](https://developer.android.com/ndk/guides/concepts)
- [JNI tips](https://developer.android.com/training/articles/perf-jni)

## Erratum

#### Acquiring AAssetManager (OUTDATED AND UNNECESSARILY COMPLICATED)
~~AAssetManager is a struct that provides access to an Android application's raw assets and is a wrapper to the
low-level native implementation of the Java AssetManager. Acquiring a pointer to a valid AAssetManager
requires a bit of work. All the moving parts necessary are listed here:~~
- ~~In C++, there exists a JNI exported method called `cacheAssetManager()` which, among the default JNI parameters,
  also takes in `jobject asset_manager`, which represents a reference to a Java-side AssetManager. This function uses the `JNIEnv` pointer (one of the default JNI
  parameters) to create a global reference to the AssetManager through `JNIEnv.NewGlobalRef()`. This new global
  reference ensures that an object being used by native code is not garbage collected by the JVM. This new global
  reference is stored as a global variable within the C++ code. Luckily, it is of type `jobject`, which does
  not facilitate itself for easy abuse.~~
- ~~`OpenGLScenesApplication.kt` is responsible for the the Java-side of the transaction. The class declares an external function called
  `cacheAssetManager()` which hooks into the C++ method through `System.loadLibrary("name-of-native-library-with-method")`.
  In the `onCreate()` method of the application, it uses this function to pass a reference to an AssetManager.~~
- ~~In `android_main()`, we utilize our global `jobject assetManagerRef` to access a
  C++-side AAssetManager. We acquire this pointer by calling the JNI function `AAssetManager_fromJava()`, which
  is defined in `#include  <android/asset_manager_jni.h>` and takes in a `JNIEnv` pointer and the AssetManager
  jobject reference. However, before we can make use of `AssetManager_fromJava()` we have to know that, under the hood,
  this function is acquiring classes/fields/methods from Java, [which requires that we be on the main thread](#general-tips).
  If we are not, we will not be able to acquire the AssetManager and our application will crash. So we utillize the
  `android_app` parameter passed into `android_main()` to attach to the main thread & get a corresponding `JNIEnv` pointer.
  We use that `JNIEnv` pointer and the global `jobject assetManagerRef` to acquire an AAssetManager pointer from
  `AAssetManager_fromJava()`. Then, finally, we detach from the main thread to continue our work. Our C++ code now has
  access to an `AAssetManager` pointer that can be used to access our Android project's assets folder, as well as any asset
  within it.~~
- ~~When the C++ code is tearing down or the AssetManager is no longer needed, `JNIEnv.DeleteGlobalRef()` may be called
  to release the AssetManager global reference and allow it to be GC'd by the JVM.~~