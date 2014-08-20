//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_BETA_H
#define INCLUDED_BETA_H

#ifndef INCLUDED_STATEMACHINE_H
#include "StateMachine.h"
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif 

#ifndef INCLUDED_QUEUE
#include <queue>
#define INCLUDED_QUEUE
#endif 

#include "Asset.h"

//#define FX_GLOW

namespace corvid {
  struct WorkQueue;
  struct TimerReactor;
}

namespace pronghorn {
  struct VertexShader;
  struct FragmentShader;
  struct Program;
  struct Framebuffer;
  struct Texture;
  struct VertexBufferObject;
}

//------------------------------------------------------------------------------

namespace beta {

//------------------------------------------------------------------------------

struct Tick {};
struct Render {};

struct GlfwKey         { int key; int scancode; int action; int mods; };
struct GlfwMouseButton { int button; int action; int mods; float x; float y;};
struct GlfwMouseMove   { double x; double y; };
struct GlfwMouseScroll { double x; double y; };

//------------------------------------------------------------------------------

typedef corvid::Machine<
  Tick, 
  Render, 
  GlfwKey,
  GlfwMouseButton,
  GlfwMouseMove,
  GlfwMouseScroll
> Beta;

struct BetaMachine : public Beta
{
  BetaMachine(int width, int height, float s = 1.0f);

  // Region
  int   width;
  int   height;
  float dpiScaling;
};

//------------------------------------------------------------------------------

} // namespace

extern beta::BetaMachine* pBeta;

//------------------------------------------------------------------------------

#endif
