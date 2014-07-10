//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Framebuffer.h"
#include "Texture.h"

#include "Log.h"
#include "AssetManager.h"

#include <stdexcept>

//------------------------------------------------------------------------------

using namespace pronghorn;

//------------------------------------------------------------------------------

#ifdef GL_ERROR_CHECK
#define checkGlError(x) checkGlError_(x)
#else
#define checkGlError(x) ;
#endif

static void 
checkGlError_(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        Log::error("after %() glError (0x%)\n", op, error);
    }
}

Framebuffer::Framebuffer() 
  : fboId_     (0) {
  glGenFramebuffers(1, &fboId_);
  checkGlError("glGenFramebuffers");
}

//------------------------------------------------------------------------------

Framebuffer::~Framebuffer() {
  #ifdef GL_CLEANUP
  glDeleteFramebuffers(1, &fboId_);
  #endif
}

//------------------------------------------------------------------------------

void
Framebuffer::bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, fboId_);
  checkGlError("glBindFramebuffer");
}

//------------------------------------------------------------------------------

void
Framebuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  checkGlError("glBindFramebuffer");
}

//------------------------------------------------------------------------------

void 
Framebuffer::attach(const Texture& tex) {
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, tex, 0);
  checkGlError("glFramebufferTexture2D");
}

//------------------------------------------------------------------------------

Framebuffer::operator GLuint() const
{
  return fboId_;
}

//------------------------------------------------------------------------------
