# OpenGL Scenes (Android)

This project is an extension of another project called [OpenGLScenes](https://github.com/Lucodivo/OpenGLScenes). The main purpose is as a graphics playground for OpenGL on Android. 

[Available on Google Play](https://play.google.com/store/apps/details?id=com.inasweaterpoorlyknit.learnopengl_androidport)

## App Features
- Infinite Cube Scene
	- Rendering technique: Traditional rasterization
	- Slowly rotating cube that displays itself on itself indefinitely
	- Scroll/Flick horizontally to rotate cube about the up-axis
	- Scroll vertically to move towards or away from the cube
	- Tap to rapidly change background color

- Infinite Capsules Scene
	- Rendering technique: Raymarching
	- Move forward through a field of infinite capsules w/ a single non-attenuating light source
	- Rotation sensor influences view & forward movement direction
	- Tap screen to re-spawn light source as a forward moving object in front of you
	- Tap & hold to increase forward travel speed

- Mandelbrot Set Scene
	- Rendering technique: Specialized fragment shader
	- Mandelbrot Set rendered in real-time
	- Supports panning, zooming, and rotating

- Menger Prison Scene (Raymarching)
	- Rendering technique: Raymarching
	- Move forward through an infinitely repeating 5th iteration Menger Sponge
	- Rotation sensor influences view & forward movement direction
	- Tap & hold to increase forward travel speed

- Information/Settings Page
	- Toggle dark mode
	- Adjust Menger Prison Scene resolution scaling
	- Adjust Mandelbrot Set Scene accent color

## Documents
- [Scene List & Scene Potential Architecture](SceneListAndScenePotentialArchitecture.md)
- [Hilt](app/src/main/java/com/inasweaterpoorlyknit/learnopengl_androidport/di/Hilt.md)
  - Note: Hilt is documented for future use. If looking for examples, not much can be found here ATM.
- [Native Code / JNI / NDK](app/src/main/cpp/AndroidNativeCode.md)
- [C++ Guidelines & Reminders](app/src/main/cpp/CppGuidelinesAndReminders.md)

## Shaders
GLSL shaders are loaded as raw resources and are currently located [here](app/src/main/res/raw). 
The positives of loading as a raw resource is that we get autocomplete functionality and compile-time checking for shaders. 
The drawbacks is that organization is hard, as sub-folders are not allowed in *res/raw* and naming convention of files is fairly restricted. 
On top of that, filetypes are obscured as the file extensions do not show in a raw resource's ID (ex: *R.raw.uv_coord_vertex_shader*). 
These issues make moving shaders to the asset folder quite appealing. But, for now, this project enjoys compile-time checkin and autocomplete.

## Math
Any 3D math in this project should be assumed to be using a left-handed coordinate system with +Z pointing forward and +Y pointing up, unless stated otherwise.
For Kotlin, [kotlin-graphics/glm](https://github.com/kotlin-graphics/glm) was previously used for matrices/vectors but it contained maven dependencies that were unavailable despite no version change on my end. A custom Kotlin matrix math module was created in its stead.
All matrices are stored in column-major.

## Building (⚠IN PROGRESS⚠)
All the following information is under the assumption the project is being built within Android Studio.  

- Ensure you have the correct CMake installed by going to *Tools > SDK Manager > SDK Tools > CMake*
  - Ensure in the *SDK Manager > SDK Tools* has the "Show Package Details" option checked to allow downloading specific CMake versions.
  - The CMake version required for this project is specified in *native_scenes/build.gradle.kts* 
    - The gradle hierarchy looks like this: *android > externalNativeBuild > cmake > version*
  - Failing to have the correct CMake installed might generate an error with the message *java.lang.NullPointerException (no error message)*
- Debug builds of the *native_scenes* library are currently set to only build for the *arm64-v8a* (AArch64) ABI. 
  - This is specified and can be changed in *native_scenes/build.gradle.kts* under *android > buildTypes > debug > ndk > abiFilters*

## Screenshots

- All screenshots taken on a Samsung Galaxy S10+

![Scene List](https://github.com/Lucodivo/RepoSampleImages/blob/master/OpenGLScenes/Android/SceneList.png)
![Infinite Cube Scene](https://github.com/Lucodivo/RepoSampleImages/blob/master/OpenGLScenes/Android/InfiniteCube.png)
![Infinite Capsules Scene](https://github.com/Lucodivo/RepoSampleImages/blob/master/OpenGLScenes/Android/InfiniteCapsules.png)
![Mandelbrot Scene](https://github.com/Lucodivo/RepoSampleImages/blob/master/OpenGLScenes/Android/Mandelbrot.png)
![MengerPrison Scene](https://github.com/Lucodivo/RepoSampleImages/blob/master/OpenGLScenes/Android/MengerPrison.png)
