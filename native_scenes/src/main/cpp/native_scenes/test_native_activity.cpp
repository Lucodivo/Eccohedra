#include "test_native_activity.h"

struct Engine {
  ASensorManager *sensorManager;
  const ASensor *gameRotationSensor;
  ASensorEventQueue *sensorEventQueue;
  GLEnvironment glEnv;
};

static s32 handleInput(android_app *app, AInputEvent *event);
static void handleAndroidCmd(android_app *app, s32 cmd);

/**
 * This is the main entry point of a native application that is using android_native_app_glue.
 * It runs in its own thread, with its own event loop for receiving input events and doing other things.
 */
void android_main(android_app *app) {
  BeginProfile();

  Engine engine{0};

  app->userData = &engine;
  app->onAppCmd = handleAndroidCmd;
  app->onInputEvent = handleInput;

  {
    TimeBlock("Acquire Android Resource Managers")
    assetManager_GLOBAL = app->activity->assetManager;
    // TODO: ASensorManager_getInstance() is deprecated. Use ASensorManager_getInstanceForPackage("foo.bar.baz");
    engine.sensorManager = ASensorManager_getInstance();
    engine.gameRotationSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
                                                                ASENSOR_TYPE_GAME_ROTATION_VECTOR);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, app->looper,
                                                              LOOPER_ID_USER, nullptr, nullptr);
  }

  initGLEnvironment(&engine.glEnv);
  logDeviceGLEnvironment();
#ifndef NDEBUG
  logAllAssets(assetManager_GLOBAL, app);
#endif

  while (true) {
    // Read all pending events.
    int pollResult, fileDescriptor, events;
    android_poll_source *source;
    ASensorEvent sensorEvent;
    // If paused, we will block forever waiting for events
    while ((pollResult = ALooper_pollAll(0, &fileDescriptor, &events, (void **) &source)) >= 0) {

      if (source != nullptr) {
        // This call goes to android_native_app_glue.c::process_cmd()
        // Which may, in turn, end up calling our function handleAndroidCmd()
        // through app->onAppCmd
        source->process(app, source);
      }

      if (pollResult == LOOPER_ID_USER) {
        if (engine.gameRotationSensor != nullptr && engine.sensorEventQueue != nullptr) {
          while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &sensorEvent, 1) > 0) {
            //LOGI("accelerometer: inputX=%f inputY=%f z=%f", sensorEvent.vector.x, sensorEvent.vector.y, sensorEvent.vector.z);
          }
        }
      }

      glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
      eglSwapBuffers(engine.glEnv.display, engine.glEnv.surface.handle);

      // Check if we are exiting.
      if (app->destroyRequested != 0) {
        return;
      }
    }
  }
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
              break;
            }
            case AMOTION_EVENT_ACTION_UP: {
              break;
            }
            case AMOTION_EVENT_ACTION_MOVE: {
              break;
            }
            default: {
              break;
            }
          }
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

static void handleAndroidCmd(android_app *app, s32 cmd) {
  Engine *engine = (Engine *) app->userData;
  switch (cmd) {
    case APP_CMD_SAVE_STATE: {
      break;
    }
    case APP_CMD_INIT_WINDOW: {
      updateGLSurface(&engine->glEnv, app->window);
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
      break;
    }
    case APP_CMD_RESUME: {
      break;
    }
    default:
      break;
  }
}