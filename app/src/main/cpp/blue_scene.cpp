/*
 NOTE: The OpenGL swap interval is set to 1, which requires a refresh of the display everytime swapBuffers
 In other words, throttling the app from going beyond the refresh rate of the phone is handled in eglSwapBuffers alone
 */

#include "native_scene.h"

typedef struct android_app android_app;
typedef struct android_poll_source android_poll_source;

/**
 * Our saved state data.
 */
typedef struct SceneState {
    f32 inputX;
    f32 inputY;
    StopWatch stopWatch;
} SceneState;

/**
 * Shared state for our app.
 */
typedef struct Engine {
    android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

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

/**
 * This is the main entry point of a native application that is using android_native_app_glue.
 * It runs in its own thread, with its own event loop for receiving input events and doing other things.
 */
void android_main(android_app* app) {
    initAssetManager(app);

    Engine engine{};

    memset(&engine, 0, sizeof(engine));
    app->userData = &engine;
    app->onAppCmd = handleAndroidCmd;
    app->onInputEvent = handleInput;
    engine.app = app;

    engine.sensorManager = acquireASensorManagerInstance(app);
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, app->looper, LOOPER_ID_USER, nullptr, nullptr);

    if (app->savedState != nullptr) {
        // grab saved state if available
        engine.state = *(SceneState*)app->savedState;
    }

    setupGLStartingState();
    loadAssets(engine);

    // TODO: Initialize scene
    // TODO: I need to call into portalScene(window) or something

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
                case LOOPER_ID_USER: { // user defined ALooper identifiers
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
//    f32 fps = f32(1.0 / frameStopWatch.lapInSeconds);
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
    auto eventType = AInputEvent_getType(event);
    auto eventSource = AInputEvent_getSource(event);
    switch(eventType) {
        case AINPUT_EVENT_TYPE_MOTION:
            switch(eventSource){
                case AINPUT_SOURCE_TOUCHSCREEN: {
                    int action = AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
                    switch(action){
                        case AMOTION_EVENT_ACTION_DOWN:
                            break;
                        case AMOTION_EVENT_ACTION_UP:
                            break;
                        case AMOTION_EVENT_ACTION_MOVE:
                            break;
                        default: { break; }
                    }
                    engine->state.inputX = AMotionEvent_getX(event, 0);
                    engine->state.inputY = AMotionEvent_getY(event, 0);
                    return 1;
                }
                default: { break; }
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
#ifndef NDEBUG
    logAllAssets();
#endif

    Model pyramidModelGltf;
    loadModel("models/pyramid.glb", &pyramidModelGltf);
}

void onResume(Engine* engine) {
    if (engine->accelerometerSensor != nullptr) {
        ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
        const s32 microsecondsPerSecond = 1000000;
        const s32 samplesPerSecond = 60;
        const s32 microsecondsPerSample = microsecondsPerSecond / samplesPerSecond;
        ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->accelerometerSensor, microsecondsPerSample);
    }
    engine->paused = false;
    engine->state.stopWatch.resetLap(); // we don't know how long we were paused, so reset stopwatch lap
}

void onPause(Engine* engine) {
    // Stop monitoring the accelerometer to avoid consuming battery while not being used.
    if (engine->accelerometerSensor != nullptr) {
        ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
    }
    engine->paused = true;
}

void onTerminate(Engine* engine) {
    glDeinitDisplay(&engine->display);
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
                //drawFrame(*engine);
            }
            break;
        }
        case APP_CMD_TERM_WINDOW: { // The window is being hidden or closed. Clean up.
            onTerminate(engine);
            break;
        }
        case APP_CMD_GAINED_FOCUS: { // the app's activity window has gained input focus.
            break;
        }
        case APP_CMD_LOST_FOCUS: { // the app's activity window has lost input focus.
            break;
        }
        case APP_CMD_PAUSE: { // the app's activity has been paused.
            onPause(engine);
            break;
        }
        case APP_CMD_RESUME: {
            onResume(engine);
            break;
        }
        default:
            break;
    }
}
