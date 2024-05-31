# Eccohedra
<a href="https://play.google.com/store/apps/details?id=com.inasweaterpoorlyknit.learnopengl_androidport"><img alt="Download" src="https://raw.githubusercontent.com/Lucodivo/RepoSampleImages/master/OpenGLScenes/Android/logo/logo-color-96x96.png"></a><br>
<a href="https://play.google.com/store/apps/details?id=com.inasweaterpoorlyknit.learnopengl_androidport"><img alt="Download" src="https://img.shields.io/badge/Google%20Play-%20?logo=googleplay&amp;color=grey"></a>

Playful and interactive 3D/2D graphic scenes.

## Screenshots

![Infinite Cube Scene](https://github.com/Lucodivo/RepoSampleImages/blob/master/OpenGLScenes/Android/InfiniteCube.png)
![Infinite Capsules Scene](https://github.com/Lucodivo/RepoSampleImages/blob/master/OpenGLScenes/Android/InfiniteCapsules.png)
![Mandelbrot Scene](https://github.com/Lucodivo/RepoSampleImages/blob/master/OpenGLScenes/Android/Mandelbrot.png)
![MengerPrison Scene](https://github.com/Lucodivo/RepoSampleImages/blob/master/OpenGLScenes/Android/MengerPrison.png)
![Gate Portal Scene](https://github.com/Lucodivo/RepoSampleImages/blob/master/OpenGLScenes/Android/GatePortal.png)   

## Video Demo

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/aROX7WxakuQ/0.jpg)](https://www.youtube.com/watch?v=aROX7WxakuQ)

## App Features
- Infinite Cube Scene
	- Rendering technique: Traditional rasterization
	- Slowly rotating cube that displays itself on itself indefinitely
	- Scroll/Flick horizontally to rotate cube about the up-axis
	- Scroll vertically or pinch to move towards or away from the cube
	- Tap to rapidly change background color

- Infinite Capsules Scene
	- Rendering technique: Raymarching
	- Move forward through a field of infinite capsules w/ a single non-attenuating light source
	- Rotation sensor influences view & forward movement direction
	- Tap screen to re-spawn light source as a forward moving object in front of you
	- Tap & hold to increase forward travel speed
	- "Crashing" into capsules resets all state

- Mandelbrot Set Scene
	- Rendering technique: Specialized fragment shader
	- Mandelbrot Set rendered in real-time
	- Supports panning, zooming, and rotating

- Menger Prison Scene
	- Rendering technique: Raymarching
	- Move forward through an infinitely repeating 5th iteration Menger Sponge
	- Rotation sensor influences view & forward movement direction
	- Tap & hold to increase forward travel speed
	- "Crashing" into Menger prison resets all state

- Gate Scene 
  - Rendering technique: Traditional rasterization with specialized projection matrices
  - Scroll/Flick horizontally to orbit around points of interest
  - Scroll vertically or pinch to move towards or away points of interest
  - Navigate through portals to explore 5 different scenes
  - Scrolling albedo/noise textures create interesting object materials

- Information/Settings Page
	- Adjust Menger Prison Scene resolution scaling
	- Adjust Mandelbrot Set Scene accent color

## Documentation
- [Scene List & Scene Potential Architecture](SceneListAndScenePotentialArchitecture.md)
- [Hilt](app/src/main/java/com/inasweaterpoorlyknit/scenes/di/Hilt.md)
- [Native Code / JNI / NDK](AndroidNativeCode.md)
- [C++ Conventions & Reminders](CppConventionsAndReminders.md)
- [How To Portals](HowToPortals.md)

## Kotlin Scenes 
### Shaders
GLSL shaders are loaded as raw resources and are currently located [here](app/src/main/res/raw). 
The positives of loading as a raw resource is that we get autocomplete functionality and compile-time checking for shaders. 
The drawbacks are that organization is hard, as sub-folders are not allowed in *res/raw* and naming convention of files is fairly restricted. 
On top of that, filetypes are obscured as the file extensions do not show in a raw resource's ID (ex: *R.raw.uv_coord_vertex_shader*). 
These issues make moving shaders to the asset folder quite appealing. But, for now, this project enjoys compile-time checkin and autocomplete.

### Math
Any 3D math in this project should be assumed to be using a left-handed coordinate system with +Z pointing forward and
+Y pointing up, unless stated otherwise. For Kotlin, [kotlin-graphics/glm](https://github.com/kotlin-graphics/glm) was
previously used for matrices/vectors. However, a simple Kotlin matrix math library, [NoopMathKt](https://github.com/lucodivo/NoopMathKt), 
was created to simplify dependencies. All matrices are stored in column-major.

## Native C++ Scenes
### Shaders
GLSL shaders are stored as assets, located [here](native_scenes/src/main/assets/shaders), and loaded using [AAssetManager](https://developer.android.com/ndk/reference/group/asset).
There are no similar trade-offs to consider as raw resources are not an option using the NDK. There is no autocomplete nor compile-time
checking of shader paths. However, one potential future for shaders is compiling them on the users device on the first encounter of the
shader, and separately storing compiled & linked programs, using 
[glGetProgramBinary](https://registry.khronos.org/OpenGL-Refpages/es3.0/html/glGetProgramBinary.xhtml)/[glProgramBinary](https://registry.khronos.org/OpenGL-Refpages/es3.0/html/glProgramBinary.xhtml),
for subsequent encounters.

### Asset Baker
This project contains source code for an asset-baker tool. This tool is used to bade assets required by the native scenes 
(textures, models, shaders). It massages data and exports files in a format that reduces the amount of work needed between
reading an asset and uploading it to the GPU.

### Math
A simple Kotlin matrix math library, [NoopMath](https://github.com/lucodivo/NoopMath), is used. All matrices are stored in 
column-major.

## Building
All the following information is under the assumption the project is being built within Android Studio.  

- If you downloaded the project directly from GitHub or are missing the noopmath submodule somehow, run the following git
  commands in the root directory of the project.

      git submodule init
      git submodule update

- Ensure you have the correct CMake installed by going to *Tools > SDK Manager > SDK Tools > CMake*
  - Ensure in the *SDK Manager > SDK Tools* has the "Show Package Details" option checked to allow downloading specific CMake versions.
  - The CMake version required for this project is specified in *native_scenes/build.gradle.kts* 
    - The gradle hierarchy looks like this: *android > externalNativeBuild > cmake > version*
  - Failing to have the correct CMake installed might generate an error with the message *java.lang.NullPointerException (no error message)*
- Debug builds of the *native_scenes* library are currently set to only build for the *arm64-v8a* (AArch64) ABI. 
  - This is specified and can be changed in *native_scenes/build.gradle.kts* under *android > buildTypes > debug > ndk > abiFilters*
- Although Vulkan is not currently used in this project, the Vulkan SDK is currently required for the typical build experience. Finding the Vulkan package
  via CMake allows access to all sorts of great tools. For this repository, we use it to find the _glslangValidator_ tool, which can
  then be used to validate GLSL ES shader files at compile time, instead of waiting until runtime.
  - If you don't already have it, you must download the [Vulkan SDK by LunarG](https://www.lunarg.com/vulkan-sdk/)
  - If you want to skip the validation of glsl shaders, removing any `add_dependencies({target-name} shaders-validation)` in CMakeLists.txt
    will do the trick.
- Assets must be baked with the asset_baker before the project can properly be run.
  - asset_baker source can be found in asset_baker/in the root directory of the project.
  - asset_baker has hardcoded directories that will work smoothly as long as asset_baker is runs in the root
  	directory of the entire Android project.
  - asset_baker uses a cache system that will re-bake items that have been modified since last they were baked.
  - asset_baker has a dependency on [AMD's Compressonator](https://github.com/GPUOpen-Tools/compressonator) static 
     libraries. Unfortunately, this must be compiled or found yourself. Fortunately, if you have access to Visual Studio,
  	 the repository makes it fairly simple.
    - load the Visual Studio project in Compressonator/vs2019
    - build x64 Release and Debug versions of the static library *compressonatorlib* (Compressonator_MD) 
    - ensure *target_link_libraries* for executable *asset_baker* in [asset_baker's CMakeLists.txt](asset_baker/CMakeLists.txt)
       correctly links to the location of the newly compiled compressonator static libraries.

## Special Thanks

### Dependencies
- [stb by Sean Barrett](https://github.com/nothings/stb)

### Model Textures
- [FreePBR](https://freepbr.com/)

### Environment Skyboxes
- [Calm Sea and Polluted Earth by Cuinn Herrick](https://opengameart.org/content/xonotic-skyboxes)
- [Cave by 3delyvisions](https://opengameart.org/content/elyvisions-skyboxes)
- [Yellow Cloudy by Spiney](https://opengameart.org/content/cloudy-skyboxes)
- [Interstellar by Jockum Skoglund (hipshot)](https://opengameart.org/content/interstellar-skybox)

### Educational Resources
- [Oblique View Frustum Depth Projection and Clipping by Eric Lengyel](http://www.terathon.com/lengyel/Lengyel-Oblique.pdf)
- [How scrolling textures gave Super Mario Galaxy 2 its charm by Jasper](https://www.youtube.com/watch?v=8rCRsOLiO7k)
- [Learn OpenGL by Joey de Vries](https://learnopengl.com/)
- [VkGuide by Victor Blanco](https://vkguide.dev/docs/extra-chapter/asset_system/)
