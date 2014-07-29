#include <set>
#include <vector>
#include <list>
#include <functional>
#include <thread>

#include "TimeMs.h"
#include "Params.h"
#include "Log.h"

// Structure
#include "Corvid.h"
#include "Relation.h"
#include "Population.h"

// Entities
#include "PewPew.h"
#include "Ship.h"
#include "Bonus.h"
#include "Roid.h"
#include "Text.h"
#include "Background.h"

using namespace pronghorn;
using namespace roidrage;

#include "RenderState.h"
#include "AudioState.h"
#include "OrthoCamera.h"
#include "RoidRage.h"
#include "RoidRageMenu.h"
#include "RoidRageGame.h"

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

static bool  down;
static float ratio;

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    pRoidRage->dispatch(AndroidBack());

  GlfwKey k; 
  k.key      = key; 
  k.scancode = scancode; 
  k.action   = action; 
  k.mods     = mods; 
  pRoidRage->dispatch(k);
}

static void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
  int iaction;
  if (action == GLFW_PRESS)   {
    iaction = Touch::down;
    down = true;
  }

  if (action == GLFW_RELEASE) { 
    iaction = Touch::up;
    down = false;
  }

  double x, y;
  glfwGetCursorPos(window, &x, &y);
  x *= ratio;
  y *= ratio;
  int* p  = {0};
  int ix[] = {(int)x};
  int iy[] = {(int)y};
  pRoidRage->dispatch(Touch(iaction, 0, 1, p, ix, iy));

  GlfwMouseButton mouse;
  mouse.button = button;
  mouse.action = action;
  mouse.mods = mods;
  mouse.x    = float(x);
  mouse.y    = float(y);
  pRoidRage->dispatch(mouse);
}

static void pos_callback(GLFWwindow* window, double x, double y)
{
  x *= ratio;
  y *= ratio;
  int* p  = {0};
  int ix[] = {(int)x};
  int iy[] = {(int)y};
  if (down) {
    pRoidRage->dispatch(Touch(Touch::move, 0, 1, p, ix, iy));
  }

  GlfwMouseMove mouse;
  mouse.x = x;
  mouse.y = y;
  pRoidRage->dispatch(mouse);
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
  window = glfwCreateWindow(640, 480, "RoidRage: ALPHA", NULL, NULL);
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

  ratio = 2.0f;

  Log::info("********************************************************************************");
  pRoidRage = new RoidRageMachine(width, height, ratio);
  pRoidRage->transition<RoidRageMenu>(true);
  //pRoidRage->transition<RoidRageGame>(true);
  Log::info("RoidRage Initialized (w=%, h=%, d=%)", width, height, ratio);
  Log::info("********************************************************************************");

  // Why must I do this?
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  while (!glfwWindowShouldClose(window)) {
    pRoidRage->dispatch(Tick());
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

