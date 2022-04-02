#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <glog/logging.h>

#include "engine/debug.h"
#include "playground/playground.h"

#include <GLFW/glfw3.h> // Will drag system OpenGL headers

static void glfw_error_callback(int error, const char *description)
{
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void handler(int sig)
{
  fprintf(stderr, "Error: signal %d:\n", sig);
  BT();
  exit(1);
}

void FillIoInput(GLFWwindow* window, ImGuiIO* imgui_io, Io* io) {
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    io->FeedKeyInput("w");
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    io->FeedKeyInput("s");
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    io->FeedKeyInput("a");
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    io->FeedKeyInput("d");
  
  double xpos, ypos; 
  glfwGetCursorPos(window, &xpos, &ypos);
  io->FeedCursorPos(xpos, ypos);

  int left_button_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1);
  int right_button_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
  io->FeedButtonInput(left_button_state == GLFW_PRESS, right_button_state == GLFW_PRESS);

  io->SetGuiCapturedMouse(imgui_io->WantCaptureMouse);
}


void GLAPIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                  const GLchar* message, const void* user_param) {
  return;
  LOG(ERROR) << "GLDebugMessageCallback";
  LOG(ERROR) << "-------------------------";
  LOG(ERROR) << "type : " << type;
  LOG(ERROR) << "id : " << id;
  LOG(ERROR) << "severity : " << severity;
  LOG(ERROR) << "length : " << length;
  LOG(ERROR) << "message : " << message;
  LOG(ERROR) << "-------------------------";
}
int main(int argc, char **argv)
{
  signal(SIGSEGV, handler); // install our handler
  signal(SIGABRT, handler); // install our handler

  glfwSetErrorCallback(glfw_error_callback);
  CGCHECK(glfwInit()) << "glfw Init Failed";

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only

  // Create window with graphics context
  constexpr int kScreenWidth = 1280;
  constexpr int kScreenHeight = 720;
  GLFWwindow* window = glfwCreateWindow(kScreenWidth, kScreenHeight, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
  CGCHECK(window) << "GLFW create window failed";
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGuiIO& imgui_io = ImGui::GetIO();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  const char *glsl_version = "#version 150";
  ImGui_ImplOpenGL3_Init(glsl_version);

  glewInit();

  glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);
  const std::string kConfigPath = "playground/config.pb.txt";
  Playground playground;
  playground.Init({kConfigPath, kScreenWidth, kScreenHeight, clear_color});

  while (!glfwWindowShouldClose(window)) {
    playground.BeginFrame();
    
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const Io& io = playground.io();
    Io* mutable_io = playground.mutable_io();
    FillIoInput(window, &imgui_io, mutable_io);

    playground.Gui();
    if (io.gui_captured_mouse()) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, io.left_button_pressed() ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
    playground.Update();
    playground.Render();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);

    playground.EndFrame();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
