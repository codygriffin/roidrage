//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_BETA_H
#define INCLUDED_BETA_H

#ifndef INCLUDED_BETAEVENTS_H
#include "BetaEvents.h"
#endif

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
