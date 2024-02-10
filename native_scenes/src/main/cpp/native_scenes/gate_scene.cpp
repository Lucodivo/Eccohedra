/*
 NOTE: The OpenGL swap interval is set to 1, which requires a refresh of the display everytime swapBuffers
 In other words, throttling the app from going beyond the refresh rate of the phone is handled in eglSwapBuffers alone
 */

#include "gate_scene.h"

typedef struct android_app android_app;
typedef struct android_poll_source android_poll_source;

struct InputState {
  bool panInProgress;
  vec2 lastPanPoint_screenRes;
  // Note: X is normalized to width and Y is normalized to height. That means, in portrait mode of a non-square
  // display, a horizontal pan yields a larger X than a horizontal pan of the same real world length.
  vec2 deltaPanPoint_normalized;

  bool pinchInProgress;
  int32_t pointerIDs[2];
  vec2 lastPinchPoints_screenRes[2];
  f32 lastPinchSpan_screenRes;
  vec2 deltaPinchFocusPoint_normalized;
  f32 deltaPinchSpan_normalized;

  // helps reduce unexpected panning when pinch pointers are released at slightly staggered times
  u32 panAfterPinchThrowAwayCount;
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
  Engine engine{0};
  engine.initializing = true;

  app->userData = &engine;
  app->onAppCmd = handleAndroidCmd;
  app->onInputEvent = handleInput;

  {
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
  SceneInput sceneInput = { 0 };

  if(inputState.panInProgress) {
    sceneInput.active = true;
    sceneInput.dx = inputState.deltaPanPoint_normalized[0];
    sceneInput.dy = inputState.deltaPanPoint_normalized[1];
  } else if(inputState.pinchInProgress) {
    sceneInput.active = true;
    sceneInput.dx = inputState.deltaPinchFocusPoint_normalized[0];
    sceneInput.dSpan_pinch = inputState.deltaPinchSpan_normalized;
  }

  updatePortalScene(&engine->sceneState.world, sceneInput);

  inputState.deltaPanPoint_normalized[0] = 0;
  inputState.deltaPanPoint_normalized[1] = 0;
  inputState.deltaPinchFocusPoint_normalized[0] = 0;
  inputState.deltaPinchFocusPoint_normalized[1] = 0;
  inputState.deltaPinchSpan_normalized = 0;
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
  Engine *engine = (Engine *) app->userData;
  InputState& inputState = engine->inputState;
  int32_t eventType = AInputEvent_getType(event);
  int32_t eventSource = AInputEvent_getSource(event);
  switch (eventType) {
    case AINPUT_EVENT_TYPE_MOTION:
      switch (eventSource) {
        case AINPUT_SOURCE_TOUCHSCREEN: {
          u32 pointerCount = AMotionEvent_getPointerCount(event);
          int action = AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
          if(pointerCount == 1) {
            vec2 pointerPos = vec2{AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0)};
            switch (action) {
              case AMOTION_EVENT_ACTION_DOWN: {
                inputState.lastPanPoint_screenRes = pointerPos;
                inputState.deltaPanPoint_normalized = vec2{.0f, .0f};
                inputState.panInProgress = true;
                break;
              }
              case AMOTION_EVENT_ACTION_UP: {
                inputState.lastPanPoint_screenRes = vec2{.0f, .0f};
                inputState.deltaPanPoint_normalized = vec2{.0f, .0f};
                inputState.panInProgress = false;
                break;
              }
              case AMOTION_EVENT_ACTION_MOVE: {
                inputState.pinchInProgress = false;
                if(!inputState.panInProgress) { // pan was interrupted by a pinch, refresh state.
                  inputState.lastPanPoint_screenRes = pointerPos;
                  inputState.deltaPanPoint_normalized = vec2{.0f, .0f};
                  inputState.panAfterPinchThrowAwayCount = 2;
                  inputState.panInProgress = true;
                  break;
                }
                if(inputState.panAfterPinchThrowAwayCount > 0) {
                  // Two fingers don't always leave the screen at the same time.
                  // This buffer is an attempt to prevent accidental pans when one finger leaves first.
                  inputState.panAfterPinchThrowAwayCount -= 1;
                  inputState.lastPanPoint_screenRes = pointerPos;
                  break;
                }
                vec2 deltaPan = pointerPos - inputState.lastPanPoint_screenRes;
                inputState.lastPanPoint_screenRes = pointerPos;
                inputState.deltaPanPoint_normalized = vec2{
                    deltaPan[0] / (f32) engine->glEnv.surface.width,
                    deltaPan[1] / (f32) engine->glEnv.surface.height
                };
                break;
              }
              default: {
                break;
              }
            }
          } else if(pointerCount == 2) { // prepare pinch to zoom input
            switch (action) {
              struct LOCAL_FUNCS {
                static void initPinchPointers(AInputEvent* event, InputState& inputState) {
                  inputState.pinchInProgress = true;
                  inputState.panInProgress = false;
                  inputState.pointerIDs[0] = AMotionEvent_getPointerId(event, 0);
                  inputState.pointerIDs[1] = AMotionEvent_getPointerId(event, 1);
                  inputState.lastPinchPoints_screenRes[0] = vec2{AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0)};
                  inputState.lastPinchPoints_screenRes[1] = vec2{AMotionEvent_getX(event, 1), AMotionEvent_getY(event, 1)};
                  inputState.lastPinchSpan_screenRes = magnitude(inputState.lastPinchPoints_screenRes[1] - inputState.lastPinchPoints_screenRes[0]);
                  inputState.deltaPinchFocusPoint_normalized[0] = 0;
                  inputState.deltaPinchFocusPoint_normalized[1] = 0;
                  inputState.deltaPinchSpan_normalized = 0;
                }
              };
              case AMOTION_EVENT_ACTION_POINTER_DOWN: {
                LOCAL_FUNCS::initPinchPointers(event, inputState);
                break;
              }
              case AMOTION_EVENT_ACTION_POINTER_UP: {
                inputState.pinchInProgress = false;
                inputState.pointerIDs[0] = -1;
                inputState.pointerIDs[1] = -1;
                inputState.lastPinchPoints_screenRes[0] = vec2{0,0};
                inputState.lastPinchPoints_screenRes[1] = vec2{0,0};
                inputState.lastPinchSpan_screenRes = 0;
                inputState.deltaPinchFocusPoint_normalized[0] = 0;
                inputState.deltaPinchFocusPoint_normalized[1] = 0;
                inputState.deltaPinchSpan_normalized = 0;
                break;
              }
              case AMOTION_EVENT_ACTION_MOVE: {
                u32 pointerId0 = AMotionEvent_getPointerId(event, 0);
                u32 pointerId1 = AMotionEvent_getPointerId(event, 1);
                vec2 pinchPointA;
                vec2 pinchPointB;
                if(pointerId0 == inputState.pointerIDs[0] && pointerId1 == inputState.pointerIDs[1]) {
                  pinchPointA = vec2{AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0)};
                  pinchPointB = vec2{AMotionEvent_getX(event, 1), AMotionEvent_getY(event, 1)};
                } else if(pointerId0 == inputState.pointerIDs[1] && pointerId1 == inputState.pointerIDs[0]){
                  assert(pointerId0 == inputState.pointerIDs[1]);
                  assert(AMotionEvent_getPointerId(event, 1) == inputState.pointerIDs[0]);
                  pinchPointA = vec2{AMotionEvent_getX(event, 1), AMotionEvent_getY(event, 1)};
                  pinchPointB = vec2{AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0)};
                } else {
                  // The current pointers are different than the start of this pinch action.
                  // Reinitialize the pinch with the current values as the starting state.
                  LOCAL_FUNCS::initPinchPointers(event, inputState);
                  break;
                }

                f32 pinchSpan = magnitude(pinchPointB - pinchPointA);
                f32 deltaPinchSpan = pinchSpan - inputState.lastPinchSpan_screenRes;

                vec2 pinchFocus = (pinchPointA + pinchPointB) / 2.0f;
                vec2 lastPinchFocus = (inputState.lastPinchPoints_screenRes[0] + inputState.lastPinchPoints_screenRes[1]) / 2.0f;
                vec2 deltaPinchFocus = pinchFocus - lastPinchFocus;

                s32 normalizedDimension = engine->glEnv.surface.width > engine->glEnv.surface.height ? engine->glEnv.surface.width : engine->glEnv.surface.height;
                inputState.deltaPinchSpan_normalized = deltaPinchSpan / (f32)normalizedDimension;
                inputState.deltaPinchFocusPoint_normalized = vec2{
                    deltaPinchFocus[0] / (f32) engine->glEnv.surface.width,
                    deltaPinchFocus[1] / (f32) engine->glEnv.surface.height
                };

                inputState.lastPinchPoints_screenRes[0] = pinchPointA;
                inputState.lastPinchPoints_screenRes[1] = pinchPointB;
                inputState.lastPinchSpan_screenRes = pinchSpan;
                break;
              }
              default: {
                break;
              }
            }
          } else {} // Do nothing if more than 2 pointers providing motion events.
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
      updateGLSurface(&engine->glEnv, app->window);
      updatePortalSceneWindow(&engine->sceneState.world, engine->glEnv.surface.width,
                        engine->glEnv.surface.height);
      engine->initializing = false;
    }
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
