//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Text.h"

#include "RoidRage.h"
#include "RenderState.h"
#include "OrthoCamera.h"
#include "Systems.h"

#include "Log.h"
#include <cmath>

//------------------------------------------------------------------------------

using namespace roidrage;
using namespace pronghorn;

//------------------------------------------------------------------------------

Text::Text(float x, float y)
: Quad          (pRoidRage->pFont.get(), pRoidRage->pCharProgram.get(), pRoidRage->pQuadVboPos.get())
, TimeDelta     ()
, TimeMs        (TimeMs::now())
, Position      ()
, Expiration    (1000)
{
  Position::pos  = glm::vec2(x, y);
  Position::avel = 0.0f;
  getRenderPass().color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

//------------------------------------------------------------------------------

Text::~Text() {
}

static float cosInterpolation(float y1, float y2, float mu)
{
  const float pi = 3.14159265f;
  float mu2;

  mu2 = (1.0f-cos(mu*pi))/2.0f;
  return(y1*(1.0f-mu2)+y2*mu2);
}


//------------------------------------------------------------------------------

// TODO generalize interpolation
void
Text::fade() {
  float e = expiration();
  if (e < 0.5f) {
    getRenderPass().color.w = cosInterpolation(0.0f, 1.0f, 2.0f *  e);
  } else {
    getRenderPass().color.w = cosInterpolation(1.0f, 0.0f, 2.0f * (e-0.5));
  }
  //pronghorn::Log::debug("Fading text: %", text_.c_str()); //TODO Log::spam
}

//------------------------------------------------------------------------------

void 
Text::preRender(OrthoCamera* pCam) {
  fade();
};

//------------------------------------------------------------------------------

void 
Text::onRender(OrthoCamera* pCam) {
  renderText(&getRenderPass(), 
             pos.x,
             pos.y,
             64.0f,       
             getText().c_str());
};

//------------------------------------------------------------------------------

void 
Text::postRender(OrthoCamera* pCam) {
  //pronghorn::Log::debug("Rendered text: %", text_.c_str()); //TODO Log::spam
};

//------------------------------------------------------------------------------
