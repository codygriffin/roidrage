//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_TEXTURE_H
#define INCLUDED_TEXTURE_H

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

//------------------------------------------------------------------------------

class Texture {
public:
  Texture(GLsizei       width, 
          GLsizei       height, 
          GLenum        fmt, 
          const GLvoid* pData);
 ~Texture();

  void bind(GLint unit = -1) const;
  void unbind();
  void allocBytes(const GLvoid* pData);
  void updateBytes(GLint   xoffset, 
                   GLint   yoffset, 
                   GLsizei width, 
                   GLsizei height, 
                   const GLvoid* pData);

  GLsizei width()      const;
  GLsizei height()     const;
  GLint   activeUnit() const;
  operator GLuint()    const;  

private:
  GLuint     texId_;
  GLsizei    width_;
  GLsizei    height_;
  GLenum     fmt_;
  GLenum     type_;

  mutable GLint activeUnit_;

public:
  static const GLenum ALPHA           = GL_ALPHA;
  static const GLenum RGB             = GL_RGB;
  static const GLenum RGBA            = GL_RGBA;
  static const GLenum LUMINANCE       = GL_RED;
  static const GLenum LUMINANCE_ALPHA = GL_RG;

  static const GLenum UNSIGNED_BYTE   = GL_UNSIGNED_BYTE; 
  //static const GLenum UNSIGNED_BYTE   = GL_UNSIGNED_SHORT_5_6_5
  //static const GLenum UNSIGNED_BYTE   = GL_UNSIGNED_SHORT_4_4_4_4
  //static const GLenum UNSIGNED_BYTE   = GL_UNSIGNED_SHORT_5_5_5_1
}; 

//------------------------------------------------------------------------------

} // namespaces

//------------------------------------------------------------------------------

#endif
