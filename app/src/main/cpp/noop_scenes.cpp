#include "noop_scenes.h"

#define VIEWPORT_INIT_WIDTH 1920
#define VIEWPORT_INIT_HEIGHT 1080

void loadGLFW();
GLFWwindow* createWindow();
void initializeGLAD();
void initializeImgui(GLFWwindow* window);

int main()
{
  loadGLFW();
  GLFWwindow* window = createWindow();
  initializeGLAD();
  initializeInput(window);
  initializeImgui(window);
  portalScene(window);
  glfwTerminate(); // clean up gl resources
  return 0;
}

void loadGLFW()
{
  int loadSucceeded = glfwInit();
  if (loadSucceeded == GLFW_FALSE)
  {
    std::cout << "Failed to load GLFW" << std::endl;
    exit(-1);
  }
}

void initializeGLAD()
{
  // intialize GLAD to help manage function pointers for OpenGL
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    exit(-1);
  }
}

GLFWwindow* createWindow()
{
  // set what kind of window desired
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL version x._
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // OpenGL version _.x
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

#if MULTI_CAMPLING_ON
  glfwWindowHint(GLFW_SAMPLES, 4);
#endif

  // create window
  GLFWwindow* window = glfwCreateWindow(VIEWPORT_INIT_WIDTH, // int Width
                                        VIEWPORT_INIT_HEIGHT, // int Height
                                        "Noop Scenes", // const char* Title
                                        NULL, // GLFWmonitor* Monitor: Specified for which monitor for fullscreen, NULL for windowed mode
                                        NULL); // GLFWwindow* Share: window to share resources with
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    exit(-1);
  }

  glfwMakeContextCurrent(window);
  enableCursor(window, false);

  return window;
}

void initializeImgui(GLFWwindow* window) {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(NULL);
}