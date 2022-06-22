/*
 NOTE: The OpenGL swap interval is set to 1, which requires a refresh of the display everytime swapBuffers
 In other words, throttling the app from going beyond the refresh rate of the phone is handled in eglSwapBuffers alone
 */

#include <memory> // memset
#include <jni.h> // Jave Native Interface: Defines communication between Java and Cpp
#include <cassert> // asserts
#include <chrono>

#include <EGL/egl.h> // interface between OpenGL ES and underlying native platform window system
#include <GLES3/gl3.h> // OpenGL ES 3.0

#include <android/sensor.h> // Used for acquiring accelerometer sensor and corresponding event queue
#include <android/log.h> // Android logging
#include <android_native_app_glue.h> // Google's glue between android and native
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <dlfcn.h> // Android dynamic library utility functions

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>

const char* NATIVE_ACTIVITY_NAME = "native-activity-blue";
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, NATIVE_ACTIVITY_NAME, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, NATIVE_ACTIVITY_NAME, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, NATIVE_ACTIVITY_NAME, __VA_ARGS__))

#include "noop_types.h"
#include "noop_math.h"
#include "android_platform.cpp"
#include "util.h"
#include "shader_types_and_constants.h"
#include "vertex_attributes.h"
#include "vertex_attributes.cpp"
#include "model.h"

typedef struct android_app android_app;
typedef struct android_poll_source android_poll_source;

global_variable jobject assetManagerJNIGlobalRef;

/**
 * Our saved state data.
 */
typedef struct SceneState {
    f32 inputX;
    f32 inputY;
} SceneState;

/**
 * Shared state for our app.
 */
typedef struct Engine {
    android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;
    AAssetManager* assetManager;

    bool paused;
    GLDisplay display;
    SceneState state;
} Engine;

static void drawFrame(const Engine &engine);
static s32 handleInput(android_app* app, AInputEvent* event);
static void handleAndroidCmd(android_app* app, s32 cmd);
void loadAssets(const Engine& engine);
void setupGLStartingState();

// The VM calls JNI_OnLoad when the native library is loaded (ex: System.loadLibrary)
// JNI_OnLoad must return the JNI version needed by the native library.
//JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
//    jvm = vm;
//    return JNI_VERSION_1_6;
//}

extern "C" JNIEXPORT void JNICALL
Java_com_inasweaterpoorlyknit_learnopengl_1androidport_OpenGLScenesApplication_cacheAssetManager(JNIEnv *env, jobject thiz, jobject asset_manager) {
    // IMPORTANT: Create a global reference to ensure the asset manager does not get GC'd on the Java side
    assetManagerJNIGlobalRef = env->NewGlobalRef(asset_manager);
}

/**
 * This is the main entry point of a native application that is using android_native_app_glue.
 * It runs in its own thread, with its own event loop for receiving input events and doing other things.
 */
void android_main(android_app* app) {
    Engine engine{};

    memset(&engine, 0, sizeof(engine));
    app->userData = &engine;
    app->onAppCmd = handleAndroidCmd;
    app->onInputEvent = handleInput;
    engine.app = app;

    // TODO: Acquiring resources seems to attach/detach from main thread multiple times,
    //  can we merge all and only attach once?
    engine.sensorManager = acquireASensorManagerInstance(app);
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, app->looper, LOOPER_ID_USER, nullptr, nullptr);
    { // get asset manager
        // IMPORTANT: AAssetManager_fromJava() relies on JNIEnv->GetClass() and JNIEnv->GetFieldId(),
        // IMPORTANT: both of which REQUIRE being called on the main thread.
        JNIEnv* mainJNIEnv = attachToMainThread(app);
        engine.assetManager = AAssetManager_fromJava(mainJNIEnv, assetManagerJNIGlobalRef);
        detachFromMainThread(app);
    }

    if (app->savedState != nullptr) {
        // grab saved state if available
        engine.state = *(SceneState*)app->savedState;
    }

    setupGLStartingState();
    loadAssets(engine);

    while (true) {

        // Read all pending events.
        int pollResult, fileDescriptor, events;
        android_poll_source* source;
        ASensorEvent sensorEvent;
        // If paused, we will block forever waiting for events
        while ((pollResult = ALooper_pollAll(engine.paused ? -1 : 0, &fileDescriptor, &events, (void**)&source)) >= 0) {

            if (source != nullptr) { source->process(app, source); }

            if (app->destroyRequested) {
                glDeinitDisplay(&engine.display);
                return;
            }

            switch(pollResult) {
                case LOOPER_ID_USER: {
                    if (engine.accelerometerSensor != nullptr) {
                        while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &sensorEvent, 1) > 0) {
//                            LOGI("accelerometer: inputX=%f inputY=%f z=%f", sensorEvent.acceleration.x, sensorEvent.acceleration.y, sensorEvent.acceleration.z);
                        }
                    }
                }
                default: {}
            }
        }

        if (!engine.paused) {
            // TODO: Update scene

            drawFrame(engine);
        }
    }
}

static void drawFrame(const Engine &engine) {
    if (engine.display.handle == EGL_NO_DISPLAY){ return; }
    func_persist StopWatch frameStopWatch = createStopWatch();
    updateStopWatch(&frameStopWatch);
    f32 fps = f32(1.0 / frameStopWatch.deltaSeconds);
//    LOGI("FPS: %f", fps);

    // Just fill the screen with a color.
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // "eglSwapBuffers performs an implicit flush operation on the context bound to surface before swapping"
    //
    eglSwapBuffers(engine.display.handle, engine.display.surface);
}

static s32 handleInput(android_app* app, AInputEvent* event) {
    auto* engine = (Engine*)app->userData;
    switch(AInputEvent_getType(event)) {
        case AINPUT_EVENT_TYPE_MOTION: {
            engine->paused = false;
            engine->state.inputX = AMotionEvent_getX(event, 0);
            engine->state.inputY = AMotionEvent_getY(event, 0);
            return 1;
        }
        case AINPUT_EVENT_TYPE_KEY: { break; }
        case AINPUT_EVENT_TYPE_FOCUS: { break; }
        default: { break; }
    }
    return 0;
}

void setupGLStartingState() {
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}

void loadAssets(const Engine& engine) {
    AAssetManager* const assetManager = engine.assetManager;
    AAssetDir* modelsDir = AAssetManager_openDir(assetManager, "models");
    const char* filename;

    AAsset* pyramidAsset = AAssetManager_open(assetManager, "models/pyramid.glb", AASSET_MODE_BUFFER);

    const off_t pyramidLength = AAsset_getLength(pyramidAsset);
    const void* pyramidBuffer = AAsset_getBuffer(pyramidAsset);
    Model pyramidModelGltf;
    loadModel(pyramidBuffer, pyramidLength, &pyramidModelGltf);

    AAsset_close(pyramidAsset);

    while((filename = AAssetDir_getNextFileName(modelsDir)) != NULL) {
        // TODO:
        LOGI("model found: %s", filename);
    }
}

/**
 * Process the next main command.
 */
static void handleAndroidCmd(android_app* app, s32 cmd) {
    Engine* engine = (Engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE: {
            // The system has asked us to save our current state
            // The glue code will clean up our malloc...
            //  - before APP_CMD_SAVE_STATE
            //  - after APP_CMD_RESUME
            //  - when the app is destroyed
            app->savedState = malloc(sizeof(SceneState));
            *((SceneState *) app->savedState) = engine->state;
            app->savedStateSize = sizeof(SceneState);
            break;
        }
        case APP_CMD_INIT_WINDOW: {
            // The window is being shown, get it ready.
            if (engine->app->window != nullptr) {
                engine->display = glInitDisplay(app->window);
                logDeviceGLEnvironment();
                drawFrame(*engine); // TODO: Is this necessary?
            }
            break;
        }
        case APP_CMD_TERM_WINDOW: {
            // The window is being hidden or closed. Clean up.
            engine->paused = true;
            glDeinitDisplay(&engine->display);
            break;
        }
        case APP_CMD_GAINED_FOCUS: {
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != nullptr) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
                const s32 microsecondsPerSecond = 1000000;
                const s32 samplesPerSecond = 60;
                const s32 microsecondsPerSample = microsecondsPerSecond / samplesPerSecond;
                ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->accelerometerSensor, microsecondsPerSample);
            }
            break;
        }
        case APP_CMD_LOST_FOCUS: {
            // Stop monitoring the accelerometer to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != nullptr) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
            }
            engine->paused = true;
            drawFrame(*engine);
            break;
        }
        default:
            break;
    }
}
