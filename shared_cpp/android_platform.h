#pragma once

#include <android/sensor.h> // Used for acquiring accelerometer sensor and corresponding event queue
#include <android/log.h> // Android logging
#include <android_native_app_glue.h> // Google's glue between android and native
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#define APP_NAME "opengl-scenes"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, APP_NAME, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, APP_NAME, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, APP_NAME, __VA_ARGS__))

struct Asset {
  const char *filePath;
  AAsset *androidAsset;
  const void *buffer;
  std::size_t bufferLengthInBytes;

  Asset(AAssetManager* assetManager, const char *filePath);
  ~Asset();

  bool success();
};

std::vector<std::string> assetsGetChildrenFilesAndDirs(android_app *app, const char *dir);
void logAllAssets(AAssetManager* assetManager, android_app *app);
JNIEnv *attachToMainThread(android_app *app);
void detachFromMainThread(android_app *app);
ASensorManager *acquireASensorManagerInstance(android_app *app);