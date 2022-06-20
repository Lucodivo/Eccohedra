/*
 NOTE: Drawing is throttled to the screen update rate, so sleeping is not necessary
 */

#include <memory> // memset
#include <jni.h> // Jave Native Interface: Defines communication between Java and Cpp
#include <cassert> // asserts

#include <EGL/egl.h> // interface between OpenGL ES and underlying native platform window system
#include <GLES/gl.h> // OpenGL ES (Embedded Systems)

#include <android/sensor.h> // Used for acquiring accelerometer sensor and corresponding event queue
#include <android/log.h> // Android logging
#include <android_native_app_glue.h> // Google's glue between android and native

#include <dlfcn.h> // Android dynamic library utility functions

const char* NATIVE_ACTIVITY_NAME = "native-activity-blue";
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, NATIVE_ACTIVITY_NAME, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, NATIVE_ACTIVITY_NAME, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, NATIVE_ACTIVITY_NAME, __VA_ARGS__))

#include "android_platform.cpp"

#include <stb_image.h>

typedef struct android_app android_app;
typedef struct android_poll_source android_poll_source;

/**
 * Our saved state data.
 */
typedef struct EngineState {
    float inputX;
    float inputY;
} EngineState;

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
    EngineState state;
} Engine;

static void drawFrame(const Engine &engine);
static int32_t handleInput(android_app* app, AInputEvent* event);
static void handleAndroidCmd(android_app* app, int32_t cmd);

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue. It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(android_app* app) {
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
        engine.state = *(EngineState*)app->savedState;
    }

    while (true) {
        // Read all pending events.
        int pollResult;
        int fileDescriptor;
        int events;
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
                            LOGI("accelerometer: inputX=%f inputY=%f z=%f", sensorEvent.acceleration.x, sensorEvent.acceleration.y, sensorEvent.acceleration.z);
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

    // Just fill the screen with a color.
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(engine.display.handle, engine.display.surface);
}

static int32_t handleInput(android_app* app, AInputEvent* event) {
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
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);
}

/**
 * Process the next main command.
 */
static void handleAndroidCmd(android_app* app, int32_t cmd) {
    Engine* engine = (Engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE: {
            // The system has asked us to save our current state
            // The glue code will clean up our malloc...
            //  - before APP_CMD_SAVE_STATE
            //  - after APP_CMD_RESUME
            //  - when the app is destroyed
            app->savedState = malloc(sizeof(EngineState));
            *((EngineState *) app->savedState) = engine->state;
            app->savedStateSize = sizeof(EngineState);
            break;
        }
        case APP_CMD_INIT_WINDOW: {
            // The window is being shown, get it ready.
            if (engine->app->window != nullptr) {
                engine->display = glInitDisplay(app->window);
                logDeviceGLEnvironment();
                setupGLStartingState();
                drawFrame(*engine);
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
                const int32_t microsecondsPerSecond = 1000000;
                const int32_t samplesPerSecond = 60;
                const int32_t microsecondsPerSample = microsecondsPerSecond / samplesPerSecond;
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
