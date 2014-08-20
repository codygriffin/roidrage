#include <set>
#include <vector>
#include <list>
#include <functional>
#include <thread>

#include "Log.h"

#include "Beta.h"
#include "BetaGame.h"

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

using namespace pronghorn;
using namespace beta;

static bool  down;
static float ratio;

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  GlfwKey k; 
  k.key      = key; 
  k.scancode = scancode; 
  k.action   = action; 
  k.mods     = mods; 
  pBeta->dispatch(k);
}

static void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  x *= ratio;
  y *= ratio;
  GlfwMouseButton mouse;
  mouse.button = button;
  mouse.action = action;
  mouse.mods = mods;
  mouse.x    = float(x);
  mouse.y    = float(y);
  pBeta->dispatch(mouse);
}

static void pos_callback(GLFWwindow* window, double x, double y)
{
  x *= ratio;
  y *= ratio;
  GlfwMouseMove mouse;
  mouse.x = x;
  mouse.y = y;
  pBeta->dispatch(mouse);
}

static void scroll_callback(GLFWwindow* window, double x, double y)
{
  GlfwMouseScroll mouse;
  mouse.x = x;
  mouse.y = y;
  pBeta->dispatch(mouse);
}

int main(void)
{
  GLFWwindow* window;
  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
      exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(1920, 1080, "Beta: ALPHA", NULL, NULL);
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  if (!window)
  {
      glfwTerminate();
      exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, mouse_callback);
  glfwSetCursorPosCallback(window, pos_callback);
  glfwSetScrollCallback(window, scroll_callback);

  ratio = 2.0f;

  Log::info("********************************************************************************");
  pBeta = new BetaMachine(width, height, ratio);
  pBeta->transition<BetaGame>();
  Log::info("roidrage:beta Initialized (w=%, h=%, d=%)", width, height, ratio);
  Log::info("********************************************************************************");

  // Why must I do this?
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  while (!glfwWindowShouldClose(window)) {
    pBeta->dispatch(Tick());
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

