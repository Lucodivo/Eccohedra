//
// Created by Connor on 6/19/2022.
//

struct GLEnvironment {
  EGLContext context;
  EGLDisplay display;
  EGLConfig config;
  struct {
    EGLSurface handle;
    s32 width;
    s32 height;
  } surface;
};

global_variable AAssetManager *assetManager_GLOBAL = nullptr;

void initAssetManager(android_app *app);

void logAllAssets();

void openAsset(const char *assetFilePath, void *assetBuffer, u32 *assetSizeInBytes);

void glDeinit(GLEnvironment *glEnv);

void logDeviceGLEnvironment();

JNIEnv *attachToMainThread(android_app *app);

void detachFromMainThread(android_app *app);

ASensorManager *acquireASensorManagerInstance(android_app *app);

// This MUST be called before attempting to load any assets
void initAssetManager(android_app *app) {
  assetManager_GLOBAL = app->activity->assetManager;
}

struct Asset {
  const char *filePath;
  AAsset *androidAsset;
  const void *buffer;
  std::size_t bufferLengthInBytes;

  Asset(const char *filePath) {
    this->filePath = filePath;
    androidAsset = AAssetManager_open(assetManager_GLOBAL, filePath, AASSET_MODE_BUFFER);
    if (androidAsset) {
      buffer = AAsset_getBuffer(androidAsset);
      bufferLengthInBytes = AAsset_getLength(androidAsset);
    } else {
      LOGI("Failed to read asset - %s", filePath);
      buffer = nullptr;
      bufferLengthInBytes = 0;
    }
  }

  bool success() { return androidAsset != NULL; }

  ~Asset() { if (androidAsset) { AAsset_close(androidAsset); }}
};

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

void logAllAssets(android_app *app) {
  TimeFunction

  const char *filename;

  LOGI("+++ ASSETS FOUND +++");
  LOGI("\t+++++ MODELS FOUND +++++");
  AAssetDir *dir = AAssetManager_openDir(assetManager_GLOBAL, "models");
  while ((filename = AAssetDir_getNextFileName(dir)) != nullptr) {
    LOGI("\t\t model found: %s", filename);
  }

  dir = AAssetManager_openDir(assetManager_GLOBAL, "shaders");
  LOGI("\t+++++ SHADERS FOUND +++++");
  while ((filename = AAssetDir_getNextFileName(dir)) != nullptr) {
    LOGI("\t\tshader found: %s", filename);
  }

  dir = AAssetManager_openDir(assetManager_GLOBAL, "skyboxes");
  LOGI("\t+++++ SKYBOXES FOUND +++++");
  std::vector<std::string> skyBoxDirectories = assetsGetChildrenFilesAndDirs(app, "skyboxes");
  for (std::string &skybox: skyBoxDirectories) {
    LOGI("\t\tskybox found: %s", skybox.c_str());
  }

  dir = AAssetManager_openDir(assetManager_GLOBAL, "textures");
  LOGI("\t+++++ TEXTURES FOUND +++++");
  while ((filename = AAssetDir_getNextFileName(dir)) != nullptr) {
    LOGI("\t\ttexture found: %s", filename);
  }

  AAssetDir_close(dir);
}

void initGLEnvironment(GLEnvironment *glEnv) {
  EGLint format;

  glEnv->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  EGLint major, minor;
  eglInitialize(glEnv->display, &major, &minor);

  if (glEnv->display == EGL_NO_DISPLAY) {
    LOGE("Unable to initialize EGLDisplay.");
  }

  LOGI("EGLDisplay initialized for EGL %d.%d", major, minor);

  const EGLint requestedAttribs[] = {
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
      EGL_BLUE_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_RED_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_DEPTH_SIZE, 24,
      EGL_STENCIL_SIZE, 8,
      EGL_NONE
  };

  EGLint totalConfigs;
  eglChooseConfig(glEnv->display, requestedAttribs, nullptr, 0, &totalConfigs);
  std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[totalConfigs]);
  EGLint suitableConfigs;
  eglChooseConfig(glEnv->display, requestedAttribs, supportedConfigs.get(), totalConfigs,
                  &suitableConfigs);

  if (suitableConfigs <= 0) {
    LOGE("Unable to initialize EGLContext. No suitable configs");
    glEnv->context = EGL_NO_CONTEXT;
    return;
  }

  glEnv->config = supportedConfigs[0];

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib(glEnv->display, glEnv->config, EGL_NATIVE_VISUAL_ID, &format);
  const EGLint contextAttrList[] = {
      // request a context using Open GL ES 3.2
      EGL_CONTEXT_MAJOR_VERSION, 3,
      EGL_CONTEXT_MINOR_VERSION, 2,
      EGL_NONE
  };
  // TODO: Create a separate shared context for loading assets
  glEnv->context = eglCreateContext(glEnv->display, glEnv->config, EGL_NO_CONTEXT, contextAttrList);

  if (eglMakeCurrent(glEnv->display, EGL_NO_SURFACE, EGL_NO_SURFACE, glEnv->context) == EGL_FALSE) {
    LOGE("Unable to attach make EGLContext current without surface");
    return;
  }
}

void updateGLSurface(GLEnvironment *glEnv, ANativeWindow *window) {
  if(glEnv->surface.handle != EGL_NO_SURFACE) {
    eglDestroySurface(glEnv->display, glEnv->surface.handle);
  }

  glEnv->surface.handle = eglCreateWindowSurface(glEnv->display, glEnv->config, window, nullptr);

  if (glEnv->surface.handle == EGL_NO_SURFACE) {
    LOGE("Unable to create surface from ANativeWindow, EGLConfig and EGLDisplay");
    return;
  }

  if (eglMakeCurrent(glEnv->display, glEnv->surface.handle, glEnv->surface.handle,
                     glEnv->context) == EGL_FALSE) {
    LOGE("Unable to attach surface to EGLDisplay/EGLContext");
    return;
  }

  eglQuerySurface(glEnv->display, glEnv->surface.handle, EGL_WIDTH, &glEnv->surface.width);
  eglQuerySurface(glEnv->display, glEnv->surface.handle, EGL_HEIGHT, &glEnv->surface.height);
  eglSwapInterval(glEnv->display, 1);
  glViewport(0, 0, glEnv->surface.width, glEnv->surface.height);
}

/**
 * Tear down the EGL context currently associated with the glEnv.
 */
void glDeinit(GLEnvironment *glEnv) {
  if (glEnv->display != EGL_NO_DISPLAY) {
    eglMakeCurrent(glEnv->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (glEnv->context != EGL_NO_CONTEXT) {
      eglDestroyContext(glEnv->display, glEnv->context);
    }
    if (glEnv->surface.handle != EGL_NO_SURFACE) {
      eglDestroySurface(glEnv->display, glEnv->surface.handle);
    }
    eglTerminate(glEnv->display);
  }
}

void logDeviceGLEnvironment() {
  auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
  for (auto name: opengl_info) {
    auto info = glGetString(name);
    LOGI("OpenGL Info: %s", info);
  }
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
  TimeFunction

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