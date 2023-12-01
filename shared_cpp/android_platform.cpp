Asset::Asset(AAssetManager* assetManager, const char *filePath) {
  this->filePath = filePath;
  androidAsset = AAssetManager_open(assetManager, filePath, AASSET_MODE_BUFFER);
  if (androidAsset) {
    buffer = AAsset_getBuffer(androidAsset);
    bufferLengthInBytes = AAsset_getLength(androidAsset);
  } else {
    LOGI("Failed to read asset - %s", filePath);
    buffer = nullptr;
    bufferLengthInBytes = 0;
  }
}
Asset::~Asset() { if (androidAsset) { AAsset_close(androidAsset); }}
bool Asset::success() { return androidAsset != NULL; }

std::vector<std::string> assetsGetChildrenFilesAndDirs(android_app *app, const char *dir) {
  /*
   * This function is necessary as there is no standard way to get asset folders from the AAssetManager.
   * Solution posted in issue #603 of Android NDK samples github repository
   * https://github.com/android/ndk-samples/issues/603#issuecomment-629219366
   */

  std::vector<std::string> children;

  JNIEnv *env = nullptr;
  app->activity->vm->AttachCurrentThread(&env, nullptr);

  auto context_object = app->activity->clazz;
  auto getAssets_method = env->GetMethodID(env->GetObjectClass(context_object), "getAssets",
                                           "()Landroid/content/res/AssetManager;");
  auto assetManager_object = env->CallObjectMethod(context_object, getAssets_method);
  auto list_method = env->GetMethodID(env->GetObjectClass(assetManager_object), "list",
                                      "(Ljava/lang/String;)[Ljava/lang/String;");

  jstring path_object = env->NewStringUTF(dir);
  auto files_object = (jobjectArray) env->CallObjectMethod(assetManager_object, list_method,
                                                           path_object);
  env->DeleteLocalRef(path_object);

  auto length = env->GetArrayLength(files_object);
  for (int i = 0; i < length; i++) {
    jstring jstr = (jstring) env->GetObjectArrayElement(files_object, i);
    const char *filename = env->GetStringUTFChars(jstr, nullptr);
    if (filename != nullptr) {
      children.push_back(filename);
      env->ReleaseStringUTFChars(jstr, filename);
    }
    env->DeleteLocalRef(jstr);
  }

  app->activity->vm->DetachCurrentThread();

  return children;
}

void logAllAssets(AAssetManager* assetManager, android_app *app) {
  const char *filename;

  LOGI("+++ ASSETS FOUND +++");
  LOGI("\t+++++ MODELS FOUND +++++");
  AAssetDir *dir = AAssetManager_openDir(assetManager, "models");
  while ((filename = AAssetDir_getNextFileName(dir)) != nullptr) {
    LOGI("\t\t model found: %s", filename);
  }

  dir = AAssetManager_openDir(assetManager, "shaders");
  LOGI("\t+++++ SHADERS FOUND +++++");
  while ((filename = AAssetDir_getNextFileName(dir)) != nullptr) {
    LOGI("\t\tshader found: %s", filename);
  }

  dir = AAssetManager_openDir(assetManager, "skyboxes");
  LOGI("\t+++++ SKYBOXES FOUND +++++");
  std::vector<std::string> skyBoxDirectories = assetsGetChildrenFilesAndDirs(app, "skyboxes");
  for (std::string &skybox: skyBoxDirectories) {
    LOGI("\t\tskybox found: %s", skybox.c_str());
  }

  dir = AAssetManager_openDir(assetManager, "textures");
  LOGI("\t+++++ TEXTURES FOUND +++++");
  while ((filename = AAssetDir_getNextFileName(dir)) != nullptr) {
    LOGI("\t\ttexture found: %s", filename);
  }

  AAssetDir_close(dir);
}

JNIEnv *attachToMainThread(android_app *app) {
  JNIEnv *env = nullptr;
  app->activity->vm->AttachCurrentThread(&env, nullptr);
  return env;
}

void detachFromMainThread(android_app *app) {
  app->activity->vm->DetachCurrentThread();
}

/*
 * AcquireASensorManagerInstance(void)
 *    Workaround ASensorManager_getInstance() deprecation false alarm
 *    for Android-N and before, when compiling with NDK-r15
 */
ASensorManager *acquireASensorManagerInstance(android_app *app) {
  if (!app)
    return nullptr;

  typedef ASensorManager *(*PF_GETINSTANCEFORPACKAGE)(const char *name);
  void *androidHandle = dlopen("libandroid.so", RTLD_NOW);
  auto getInstanceForPackageFunc = (PF_GETINSTANCEFORPACKAGE)
      dlsym(androidHandle, "ASensorManager_getInstanceForPackage");
  if (getInstanceForPackageFunc) {
    JNIEnv *env = attachToMainThread(app);

    jclass android_content_Context = env->GetObjectClass(app->activity->clazz);
    jmethodID midGetPackageName = env->GetMethodID(android_content_Context, "getPackageName",
                                                   "()Ljava/lang/String;");
    auto packageName = (jstring) env->CallObjectMethod(app->activity->clazz, midGetPackageName);

    const char *nativePackageName = env->GetStringUTFChars(packageName, nullptr);
    ASensorManager *mgr = getInstanceForPackageFunc(nativePackageName);
    env->ReleaseStringUTFChars(packageName, nativePackageName);
    detachFromMainThread(app);
    if (mgr) {
      dlclose(androidHandle);
      return mgr;
    }
  }

  typedef ASensorManager *(*PF_GETINSTANCE)();
  auto getInstanceFunc = (PF_GETINSTANCE)
      dlsym(androidHandle, "ASensorManager_getInstance");
  // by all means at this point, ASensorManager_getInstance should be available
  assert(getInstanceFunc);
  dlclose(androidHandle);

  return getInstanceFunc();
}