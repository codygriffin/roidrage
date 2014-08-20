//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_VERTEXBUFFEROBJECT_H
#define INCLUDED_VERTEXBUFFEROBJECT_H

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

class VertexBufferObject {
public:
  VertexBufferObject(GLsizeiptr sizeBytes, const GLvoid* pData = 0, GLenum usage = STATIC);
 ~VertexBufferObject();

  void bind();
  void unbind();
  void allocBytes(GLsizeiptr sizeBytes, const GLvoid* pData, GLenum usage);
  void updateBytes(GLintptr offsetBytes, GLsizeiptr sizeBytes, const GLvoid* pData);

  GLsizeiptr sizeBytes() const;
  GLenum     usageHint() const;

private:
  GLuint     vboId_;
  GLsizeiptr sizeBytes_;
  GLenum     usage_;

public:
  static const GLenum STREAM  = GL_STREAM_DRAW;
  static const GLenum STATIC  = GL_STATIC_DRAW;
  static const GLenum DYNAMIC = GL_DYNAMIC_DRAW;

private:
  // Hackish memoization
  static GLuint currentVboId_;
}; 

//------------------------------------------------------------------------------

} // namespaces

//------------------------------------------------------------------------------

#endif
