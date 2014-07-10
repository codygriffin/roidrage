//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Frame.h"
#include "OrthoCamera.h"
#include "Program.h"
#include "VertexBufferObject.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "RoidRage.h"

using namespace roidrage;
using namespace pronghorn;

//------------------------------------------------------------------------------

Frame::Frame(int x, int y) 
  : Quad(0, 
         pRoidRage->pGlowProgram.get(), 
         pRoidRage->pQuadVboPos.get())
  , pFrameTexture_(new Texture(x, y, Texture::RGBA, 0))
  , pFbo_         (new Framebuffer()) {
  getRenderPass().pTex.push_back(pFrameTexture_.get());
  pFbo_->bind();
  pFbo_->attach(*pFrameTexture_.get());
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  Framebuffer::unbind();
}

//------------------------------------------------------------------------------
