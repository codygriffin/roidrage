//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Beta.h"
#include "Log.h"
#include "Display.h"
#include "Gl.h"
#include "Viewport.h"

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace gl;
using namespace roidrage;
using namespace beta;

//------------------------------------------------------------------------------


BetaMachine* pBeta = 0;

BetaMachine::BetaMachine(int w, int h, float s) 
  : width  (w) 
  , height (h) 
  , dpiScaling (s) 
{ 
  Display::reset(w, h, s);
  Viewport::reset(glm::vec4(0,0,w,h));

  // Some misc opengl stuff
  glDisable(GL_SCISSOR_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  Log::info("Display initialized");
}

//------------------------------------------------------------------------------
