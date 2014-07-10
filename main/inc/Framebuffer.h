//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_FRAMEBUFFER_H
#define INCLUDED_FRAMEBUFFER_H

#ifdef ANDROID
#ifndef INCLUDED_GLES_GL2_H
#define INCLUDED_GLES_GL2_H
#include <GLES2/gl2.h>
#endif
#else
#include <GLFW/glfw3.h>
#endif

//------------------------------------------------------------------------------

namespace pronghorn {

struct Texture; 

//------------------------------------------------------------------------------

struct Framebuffer {
  Framebuffer();
 ~Framebuffer();

  void bind();
  static void unbind();
  void attach(const Texture& tex);

  operator GLuint()    const;  

private:
  GLuint     fboId_;
}; 

//------------------------------------------------------------------------------

} // namespace

//------------------------------------------------------------------------------

#endif
