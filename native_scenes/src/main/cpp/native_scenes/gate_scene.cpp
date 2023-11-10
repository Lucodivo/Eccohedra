/*
 NOTE: The OpenGL swap interval is set to 1, which requires a refresh of the display everytime swapBuffers
 In other words, throttling the app from going beyond the refresh rate of the phone is handled in eglSwapBuffers alone
 */

#include "gate_scene.h"

typedef struct android_app android_app;
typedef struct android_poll_source android_poll_source;

struct InputState {
  // Position //
  vec2 lastPos_screenRes;
  // Note: X is normalized to width and Y is normalized to height. That means, in portrait mode of a non-square
  // display, a horizontal pan yields a larger X than a horizontal pan of the same real world length.
  // Not the best way but the way for now
  vec2 deltaPos_normalized;
};

struct SceneState {
  World world;
};

struct Engine {
  ASensorManager *sensorManager;
  ASensorEventQueue *sensorEventQueue;
  bool paused;
  bool initializing;
  GLEnvironment glEnv;
  SceneState sceneState;
  InputState inputState;
};

static void update(Engine *engine);

static void drawFrame(Engine *engine);

static s32 handleInput(android_app *app, AInputEvent *event);

static void handleAndroidCmd(android_app *app, s32 cmd);

void glDeinit(GLEnvironment *glEnv);

void closeActivity(ANativeActivity *activity);

void onTerminate(Engine *engine);

void onResume(android_app *app, Engine *engine);

void onPause(Engine *engine);

/**
 * This is the main entry point of a native application that is using android_native_app_glue.
 * It runs in its own thread, with its own event loop for receiving input events and doing other things.
 */
void android_main(android_app *app) {
  BeginProfile();

  Engine engine{0};
  engine.initializing = true;

  app->userData = &engine;
  app->onAppCmd = handleAndroidCmd;
  app->onInputEvent = handleInput;


  {
    TimeBlock("Acquire Android Resource Managers")
    assetManager_GLOBAL = app->activity->assetManager;
    // TODO: ASensorManager_getInstance() is deprecated. Use ASensorManager_getInstanceForPackage("foo.bar.baz");
    engine.sensorManager = ASensorManager_getInstance();
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, app->looper,
                                                              LOOPER_ID_USER, nullptr, nullptr);
  }

  initGLEnvironment(&engine.glEnv);
  logDeviceGLEnvironment();
#ifndef NDEBUG
  logAllAssets(assetManager_GLOBAL, app);
#endif
  initPortalScene(&engine.sceneState.world);

  while (true) {
    // Read all pending events.
    int pollResult, fileDescriptor, events;
    android_poll_source *source;
    ASensorEvent sensorEvent;
    // If paused, we will block forever waiting for events
    while ((pollResult = ALooper_pollAll(engine.paused ? -1 : 0, &fileDescriptor, &events,
                                         (void **) &source)) >= 0) {

      if (source != nullptr) {
        // This call goes to android_native_app_glue.c::process_cmd()
        // Which may, in turn, end up calling our function handleAndroidCmd()
        // through app->onAppCmd
        source->process(app, source);
      }

      if (pollResult == LOOPER_ID_USER) {
        if (engine.sensorEventQueue != nullptr) {
          while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &sensorEvent, 1) > 0) {
            // send events to sensor handler
          }
        }
      }

      // Check if we are exiting.
      if (app->destroyRequested != 0) {
        onTerminate(&engine);
        return;
      }
    }

    if (!engine.paused) {
      update(&engine);
      drawFrame(&engine);
    }
  }
}

static void update(Engine *engine) {
  InputState &inputState = engine->inputState;
  SceneInput sceneInput;

  sceneInput.x = inputState.deltaPos_normalized[0];
  sceneInput.y = inputState.deltaPos_normalized[1];

  updatePortalScene(&engine->sceneState.world, sceneInput);
}

static void drawFrame(Engine *engine) {
  if (engine->glEnv.surface.handle == EGL_NO_SURFACE) { return; }

  if (engine->initializing) {
    // TODO: Draw loading screen
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  } else {
    drawPortalScene(&engine->sceneState.world);
  }

  // "eglSwapBuffers performs an implicit flush operation on the context bound to surface before swapping"
  eglSwapBuffers(engine->glEnv.display, engine->glEnv.surface.handle);
}

static s32 handleInput(android_app *app, AInputEvent *event) {
  auto *engine = (Engine *) app->userData;
  auto eventType = AInputEvent_getType(event);
  auto eventSource = AInputEvent_getSource(event);
  switch (eventType) {
    case AINPUT_EVENT_TYPE_MOTION:
      switch (eventSource) {
        case AINPUT_SOURCE_TOUCHSCREEN: {
          int action = AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
          vec2 actionPos = vec2{AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0)};

          switch (action) {
            case AMOTION_EVENT_ACTION_DOWN: {
              engine->inputState.deltaPos_normalized = vec2{.0f, .0f};
              break;
            }
            case AMOTION_EVENT_ACTION_UP: {
              engine->inputState.lastPos_screenRes = vec2{.0f, .0f};
              engine->inputState.deltaPos_normalized = vec2{.0f, .0f};
              break;
            }
            case AMOTION_EVENT_ACTION_MOVE: {
              vec2 deltaPan = actionPos - engine->inputState.lastPos_screenRes;
              engine->inputState.deltaPos_normalized = vec2{
                  deltaPan[0] / (f32) engine->glEnv.surface.width,
                  deltaPan[1] / (f32) engine->glEnv.surface.height
              };
              break;
            }
            default: {
              break;
            }
          }

          engine->inputState.lastPos_screenRes = actionPos;
          return 1;
        }
        default: {
          break;
        }
      }
    case AINPUT_EVENT_TYPE_KEY: {
      break;
    }
    case AINPUT_EVENT_TYPE_FOCUS: {
      break;
    }
    default: {
      break;
    }
  }
  return 0;
}

void closeActivity(ANativeActivity *activity) { ANativeActivity_finish(activity); }

void onSavedState(android_app *app, Engine *engine) {
  // TODO: Implement saved state functionality.
  // TODO: Minor testing showed that when app->savedState is free'd is not straight forward...
}

void onWindowInit(android_app *app, Engine *engine) {
  // The window is being shown, get it ready.
  if (app->window != nullptr) {
    {
      TimeBlock("APP_CMD_INIT_WINDOW")
      updateGLSurface(&engine->glEnv, app->window);
      updateSceneWindow(&engine->sceneState.world, engine->glEnv.surface.width,
                        engine->glEnv.surface.height);
      engine->initializing = false;
    }
    EndAndPrintProfile();
  }
}

/**
 * This function is called from android_native_app_glue.c::process_cmd()
 * And that function is called when processing events (ex: source->process(app, source))
 * This function will be called on the same thread as the one processing events.
 */
static void handleAndroidCmd(android_app *app, s32 cmd) {
  Engine *engine = (Engine *) app->userData;
  switch (cmd) {
    case APP_CMD_SAVE_STATE: {
      // The system has asked us to save our current state
      // The glue code will clean up our malloc...
      //  - before APP_CMD_SAVE_STATE
      //  - after APP_CMD_RESUME
      //  - when the app is destroyed
      TimeBlock("APP_CMD_SAVE_STATE")
      onSavedState(app, engine);
      break;
    }
    case APP_CMD_INIT_WINDOW: {
      onWindowInit(app, engine);
      break;
    }
    case APP_CMD_TERM_WINDOW: { // The window is being hidden or closed. Clean up.
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
      onResume(app, engine);
      break;
    }
    default:
      break;
  }
}

void onResume(android_app *app, Engine *engine) {
  engine->paused = false;
}

void onTerminate(Engine *engine) {
  ASensorManager_destroyEventQueue(engine->sensorManager, engine->sensorEventQueue);
  deinitPortalScene(&engine->sceneState.world);
  glDeinit(&engine->glEnv);
}

void onPause(Engine *engine) {
  engine->paused = true;
}
