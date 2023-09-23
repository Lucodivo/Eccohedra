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

void initGLEnvironment(GLEnvironment *glEnv) {
  TimeFunction

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

void updateGLSurface(GLEnvironment *glEnv, EGLNativeWindowType window) {
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
  TimeFunction

  auto info = glGetString(GL_VENDOR);
  LOGI("OpenGL Info: Vendor\t- %s", info);

  info = glGetString(GL_RENDERER);
  LOGI("OpenGL Info: GPU\t- %s", info);

  info = glGetString(GL_VERSION);
  LOGI("OpenGL Info: Version\t- %s", info);

  info = glGetString(GL_EXTENSIONS);
  LOGI("OpenGL Info: Extensions", info);
  char* substrBegin = (char*)info;
  char* substrEnd = (char*)info;
  while(*substrEnd != '\0') {
    while(*substrEnd++ != ' '){}
    LOGI("\t\t%.*s", substrEnd - substrBegin, substrBegin);
    substrBegin = substrEnd;
  }
  LOGI("\t\t%s", substrBegin);
}

s64 sizeOfUniformBlock(GLuint programId, const char* name) {
  GLuint uniformBlockId = glGetProgramResourceIndex(programId, GL_UNIFORM_BLOCK, name);
  GLint blockDataSize = 0;
  glGetActiveUniformBlockiv(programId,uniformBlockId, GL_UNIFORM_BLOCK_DATA_SIZE,  &blockDataSize);
  return blockDataSize;
}