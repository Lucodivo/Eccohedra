//
// Created by Connor on 6/19/2022.
//

struct GLDisplay {
    EGLDisplay handle;
    EGLSurface surface;
    EGLContext context;
    s32 width;
    s32 height;
};

const GLDisplay NULL_GLDISPLAY = {
        EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_CONTEXT,
        -1, -1,
};

global_variable AAssetManager* assetManager_GLOBAL = nullptr;
void initAssetManager(android_app* app);
void logAllAssets();
void openAsset(const char* assetFilePath, void* assetBuffer, u32* assetSizeInBytes);
GLDisplay glInitDisplay(ANativeWindow *window);
void glDeinitDisplay(GLDisplay* display);
void logDeviceGLEnvironment();
JNIEnv* attachToMainThread(android_app* app);
void detachFromMainThread(android_app* app);
ASensorManager* acquireASensorManagerInstance(android_app* app);

// This MUST be called before attempting to load any assets
void initAssetManager(android_app* app) {
    assetManager_GLOBAL = app->activity->assetManager;
}

struct Asset {
    const char* filePath;
    AAsset* androidAsset;
    const void* buffer;
    u32 bufferLengthInBytes;

    Asset(const char* filePath) {
        this->filePath = filePath;
        androidAsset = AAssetManager_open(assetManager_GLOBAL, filePath, AASSET_MODE_BUFFER);
        buffer = AAsset_getBuffer(androidAsset);
        bufferLengthInBytes = AAsset_getLength(androidAsset);
    }

    ~Asset() { AAsset_close(androidAsset); }
};

void logAllAssets() {
    const char* filename;

    LOGI("+++++ ASSETS FOUND +++++");
    LOGI("\t+++++ MODELS FOUND +++++");
    AAssetDir* dir = AAssetManager_openDir(assetManager_GLOBAL, "models");
    while((filename = AAssetDir_getNextFileName(dir)) != nullptr) {
        LOGI("\t\t model found: %s", filename);
    }

    dir = AAssetManager_openDir(assetManager_GLOBAL, "shaders");
    LOGI("\t+++++ SHADERS FOUND +++++");
    while((filename = AAssetDir_getNextFileName(dir)) != nullptr) {
        LOGI("\t\tshader found: %s", filename);
    }

    dir = AAssetManager_openDir(assetManager_GLOBAL, "skyboxes");
    LOGI("\t+++++ SKYBOXES FOUND +++++");
    while((filename = AAssetDir_getNextFileName(dir)) != nullptr) {
        LOGI("\t\tskybox found: %s", filename);
    }

    dir = AAssetManager_openDir(assetManager_GLOBAL, "textures");
    LOGI("\t+++++ TEXTURES FOUND +++++");
    while((filename = AAssetDir_getNextFileName(dir)) != nullptr) {
        LOGI("\t\ttexture found: %s", filename);
    }
}

/**
 * Initialize an EGL context for the current display.
 * Returns display handle of EGL_NO_DISPLAY if error occurs
 */
GLDisplay glInitDisplay(ANativeWindow *window) {
    // initialize OpenGL ES and EGL
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, format;
    EGLint numConfigs;
    EGLConfig config = nullptr;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, nullptr, nullptr);

    /* Here, the application chooses the configuration it desires.
     * find the best match if possible, otherwise use the very first one
     */
    eglChooseConfig(display, attribs, nullptr,0, &numConfigs);
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    assert(supportedConfigs);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
    assert(numConfigs);
    auto i = 0;
    for (; i < numConfigs; i++) {
        auto& cfg = supportedConfigs[i];
        EGLint r, g, b, d;
        if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r)   &&
            eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
            eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b)  &&
            eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) &&
            r == 8 && g == 8 && b == 8 && d == 0 ) {

            config = supportedConfigs[i];
            break;
        }
    }
    if (i == numConfigs) {
        config = supportedConfigs[0];
    }

    if (config == nullptr) {
        LOGW("Unable to initialize EGLConfig");
        return NULL_GLDISPLAY;
    }

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    surface = eglCreateWindowSurface(display, config, window, nullptr);
    context = eglCreateContext(display, config, nullptr, nullptr);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return NULL_GLDISPLAY;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    eglSwapInterval(display, 1);

    GLDisplay result{};
    result.handle = display;
    result.context = context;
    result.surface = surface;
    result.width = w;
    result.height = h;

    return result;
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void glDeinitDisplay(GLDisplay* display) {
    if (display->handle != EGL_NO_DISPLAY) {
        eglMakeCurrent(display->handle, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (display->context != EGL_NO_CONTEXT) {
            eglDestroyContext(display->handle, display->context);
        }
        if (display->surface != EGL_NO_SURFACE) {
            eglDestroySurface(display->handle, display->surface);
        }
        eglTerminate(display->handle);
    }
    *display = NULL_GLDISPLAY;
}

void logDeviceGLEnvironment() {
    auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
    for (auto name : opengl_info) {
        auto info = glGetString(name);
        LOGI("OpenGL Info: %s", info);
    }
}

JNIEnv* attachToMainThread(android_app* app) {
    JNIEnv* env = nullptr;
    app->activity->vm->AttachCurrentThread(&env, nullptr);
    return env;
}

void detachFromMainThread(android_app* app) {
    app->activity->vm->DetachCurrentThread();
}

/*
 * AcquireASensorManagerInstance(void)
 *    Workaround ASensorManager_getInstance() deprecation false alarm
 *    for Android-N and before, when compiling with NDK-r15
 */
ASensorManager* acquireASensorManagerInstance(android_app* app) {

    if(!app)
        return nullptr;

    typedef ASensorManager *(*PF_GETINSTANCEFORPACKAGE)(const char *name);
    void* androidHandle = dlopen("libandroid.so", RTLD_NOW);
    auto getInstanceForPackageFunc = (PF_GETINSTANCEFORPACKAGE)
            dlsym(androidHandle, "ASensorManager_getInstanceForPackage");
    if (getInstanceForPackageFunc) {
        JNIEnv* env = attachToMainThread(app);

        jclass android_content_Context = env->GetObjectClass(app->activity->clazz);
        jmethodID midGetPackageName = env->GetMethodID(android_content_Context, "getPackageName", "()Ljava/lang/String;");
        auto packageName = (jstring)env->CallObjectMethod(app->activity->clazz, midGetPackageName);

        const char *nativePackageName = env->GetStringUTFChars(packageName, nullptr);
        ASensorManager* mgr = getInstanceForPackageFunc(nativePackageName);
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