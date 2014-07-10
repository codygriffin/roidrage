//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

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
        Log::info("after %() glError (0x%)\n", op, error);
    }
}

Texture::Texture(GLsizei       width, 
                 GLsizei       height, 
                 GLenum        fmt, 
                 const GLvoid* pData) 
  : texId_     (0) 
  , width_     (width)
  , height_    (height)
  , fmt_       (fmt)
  , type_      (UNSIGNED_BYTE)
  , activeUnit_(-1) 
{
  glGenTextures(1, &texId_);
  checkGlError("glGenTextures");
  bind();
  allocBytes(pData); 
}

//------------------------------------------------------------------------------

Texture::~Texture() 
{
  #ifdef GL_CLEANUP
  glDeleteTextures(1, &texId_);
  #endif
}

//------------------------------------------------------------------------------

void
Texture::bind(GLint unit) const
{
  if (unit >= 0) {
    glActiveTexture(GL_TEXTURE0 + unit);
    checkGlError("glActiveTexture");
    activeUnit_ = unit;
  }

  glBindTexture(GL_TEXTURE_2D, texId_);
  checkGlError("glBindTexture");
}

//------------------------------------------------------------------------------

void
Texture::unbind() 
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

//------------------------------------------------------------------------------

void
Texture::allocBytes(const GLvoid* pData) 
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  checkGlError("glPixelStore");

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  checkGlError("glTexParam 1");
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  checkGlError("glTexParam 2");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
  checkGlError("glTexParam 3");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
  checkGlError("glTexParam 4");

  // TODO assert that this is the current bound Texture 
  glTexImage2D(GL_TEXTURE_2D, 
               0, 
               fmt_, 
               width_, 
               height_, 
               0, 
               fmt_, 
               type_, 
               pData);
  checkGlError("glTexImage2D");
}

//------------------------------------------------------------------------------

void
Texture::updateBytes(GLint   xoffset, 
                     GLint   yoffset, 
                     GLsizei width, 
                     GLsizei height, 
                     const GLvoid* pData) 
{
  // TODO assert that this is the current bound Texture 
  glTexSubImage2D(GL_TEXTURE_2D, 
                  0, 
                  xoffset, 
                  yoffset, 
                  width_, 
                  height_, 
                  fmt_, 
                  type_, 
                  pData);
  checkGlError("glTexSubImage2D");
}

//------------------------------------------------------------------------------

GLsizei
Texture::width() const
{
  return width_;
}

//------------------------------------------------------------------------------

GLsizei
Texture::height() const
{
  return height_;
}

//------------------------------------------------------------------------------

GLint
Texture::activeUnit() const
{
  return activeUnit_;
}

//------------------------------------------------------------------------------

Texture::operator GLuint() const
{
  return texId_;
}

//------------------------------------------------------------------------------
