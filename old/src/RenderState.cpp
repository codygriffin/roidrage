//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "RenderState.h"
#include "OrthoCamera.h"
#include "Gl.h"

#include <memory>

//------------------------------------------------------------------------------

using namespace roidrage;

//------------------------------------------------------------------------------

std::unique_ptr<roidrage::OrthoCamera> RenderState::pCam_;

//------------------------------------------------------------------------------

void 
RenderState::reset(int width, int height) {
  // Initialize Viewport
  glViewport(0, 0, width, height);
  pCam_.reset(new OrthoCamera(width, height));

  glDisable(GL_SCISSOR_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);
  //glEnable(GL_DEPTH_TEST);
  //glDepthFunc(GL_LEQUAL);
}

//------------------------------------------------------------------------------
