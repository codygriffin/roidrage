//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#include "VertexBufferObject.h"

#include "Log.h"
#include <stdexcept>

//------------------------------------------------------------------------------

using namespace pronghorn;

//------------------------------------------------------------------------------

GLuint VertexBufferObject::currentVboId_ = 0;

#ifdef GL_ERROR_CHECK
#define checkGlError(x) checkGlError_(x)
#else
#define checkGlError(x) ;
#endif

static void 
checkGlError_(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        Log::info("after %() glError (%)\n", op, error);
    }
}

//------------------------------------------------------------------------------

VertexBufferObject::VertexBufferObject(GLsizeiptr   sizeBytes, 
                                      const GLvoid* pData, 
                                      GLenum        usage) 
  : vboId_    (0) 
{
  glGenBuffers(1, &vboId_);
  checkGlError("glGenBuffers");

  bind();
  allocBytes(sizeBytes, pData, usage); 
}

//------------------------------------------------------------------------------

VertexBufferObject::~VertexBufferObject() 
{
  #ifdef GL_CLEANUP
  glDeleteBuffers(1, &vboId_);
  checkGlError("glDeleteBuffers");
  #endif
}

//------------------------------------------------------------------------------

void
VertexBufferObject::bind() 
{
  //if (currentVboId_ != vboId_) {
    glBindBuffer(GL_ARRAY_BUFFER, vboId_);
    checkGlError("glBindBuffer");
  //  currentVboId_ = vboId_;
  //}
}

//------------------------------------------------------------------------------

void
VertexBufferObject::unbind() 
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  checkGlError("glBindBuffer(0)");
  currentVboId_ = 0;
}

//------------------------------------------------------------------------------

void
VertexBufferObject::allocBytes(GLsizeiptr sizeBytes, const GLvoid* pData, GLenum usage) 
{
  if (currentVboId_ != vboId_) {
    bind();
  }

  glBufferData(GL_ARRAY_BUFFER, sizeBytes, pData, usage);
  checkGlError("glBufferData");
  sizeBytes_ = sizeBytes;
  usage_     = usage;
}

//------------------------------------------------------------------------------

void
VertexBufferObject::updateBytes(GLintptr offsetBytes, GLsizeiptr sizeBytes, const GLvoid* pData) 
{
  if (currentVboId_ != vboId_) {
    bind();
  }

  glBufferSubData(GL_ARRAY_BUFFER, offsetBytes, sizeBytes, pData);
  checkGlError("glBufferSubData");
}

//------------------------------------------------------------------------------

GLsizeiptr
VertexBufferObject::sizeBytes() const
{
  return sizeBytes_;
}

//------------------------------------------------------------------------------

GLenum
VertexBufferObject::usageHint() const
{
  return usage_;
}

//------------------------------------------------------------------------------
