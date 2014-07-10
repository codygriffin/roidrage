//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Program.h"

#include "Log.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexBufferObject.h"
#include <memory>
#include <stdexcept>

#define MEMOIZE_PROGRAM_USE

//------------------------------------------------------------------------------

using namespace pronghorn;

//------------------------------------------------------------------------------

GLuint Program::currentProgId_ = -1;

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
        throw;
    }
}

Program::Program(const VertexShader&   vertexShader, 
                 const FragmentShader& fragShader,
                 const GLenum          primitiveType)
  : primitiveType_(primitiveType)
  , progId_       (0) 
{
  progId_ = glCreateProgram();
  if (!progId_) {
    Log::error("Could not create program.");
    throw;
  }

  glAttachShader(progId_, vertexShader.getId());
  checkGlError("glAttachShader(vertex)");
  glAttachShader(progId_, fragShader.getId());
  checkGlError("glAttachShader(fragment)");

  glLinkProgram(progId_);
  checkGlError("glLinkProgram");

  GLint linked = GL_FALSE;
  glGetProgramiv(progId_, GL_LINK_STATUS, &linked);
  checkGlError("glGetProgramiv");
  if (!linked) {
    GLint bufferLength = 0;
    glGetProgramiv(progId_, GL_INFO_LOG_LENGTH, &bufferLength);
    checkGlError("glGetProgramiv");
    if (bufferLength) {
      std::unique_ptr<GLchar> buffer(new GLchar[bufferLength]);
      glGetProgramInfoLog(progId_, bufferLength, 0, buffer.get());
      checkGlError("glGetProgramInfoLog");
      Log::error("Could not link program:\n%\n", buffer.get());
    }
    throw;
  }
}

//------------------------------------------------------------------------------

Program::~Program() {
  #ifdef GL_CLEANUP
  glDeleteProgram(progId_);
  #endif
}

//------------------------------------------------------------------------------

GLuint
Program::getId() const {
  return progId_;
}

//------------------------------------------------------------------------------

void
Program::use() {
  #ifdef MEMOIZE_PROGRAM_USE
  if (currentProgId_ == progId_) 
    return;
  #endif

  glUseProgram(progId_);
  checkGlError("glUseProgram");
  currentProgId_ = progId_;
}

//------------------------------------------------------------------------------

void
Program::execute(GLint offset, GLsizei length) {
  glDrawArrays(primitiveType_, offset, length);
  checkGlError("glDrawArrays");
}

//------------------------------------------------------------------------------

void
Program::attribute(const char*         pName, 
                   VertexBufferObject& vbo,
                   GLint               size,
                   GLsizei             stride,
                   GLsizei             offset) {

  GLuint attrId = glGetAttribLocation(progId_, pName);
  checkGlError("glGetAttribLocation");

  vbo.bind();

  glEnableVertexAttribArray(attrId);
  checkGlError("glEnableVertexAttribArray");

  glVertexAttribPointer(attrId, size, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
  checkGlError("glVertexAttribPointer");
}

//------------------------------------------------------------------------------

void   
Program::uniform(const char* pName,   
                 float       num) {
  GLuint uniId = glGetUniformLocation(progId_, pName);
  checkGlError("glGetUniformLocation");
  if (uniId < 0) {
    Log::error("no uniform with name \"%\" found in program", pName);
    return;
  }
  glUniform1f(uniId, num);
  checkGlError("glUniform1f");
}

//------------------------------------------------------------------------------

void   
Program::uniform(const char* pName,   
                 int         num) {
  GLuint uniId = glGetUniformLocation(progId_, pName);
  checkGlError("glGetUniformLocation");
  if (uniId < 0) {
    Log::error("no uniform with name \"%\" found in program", pName);
    return;
  }
  glUniform1i(uniId, num);
  checkGlError("glUniform1i");
}

//------------------------------------------------------------------------------

void
Program::uniform(const char*      pName,   
                 const glm::mat4& matrix) {
  GLuint uniId = glGetUniformLocation(progId_, pName);
  checkGlError("glGetUniformLocation");
  if (uniId < 0) {
    Log::error("no uniform with name \"%\" found in program", pName);
    return;
  }
  glUniformMatrix4fv(uniId, 1, GL_FALSE, &matrix[0][0]);
  checkGlError("glUniformMatrix4fv");
}

//------------------------------------------------------------------------------

void
Program::uniform(const char*      pName,   
                 const glm::vec4& vec) {
  GLuint uniId = glGetUniformLocation(progId_, pName);
  checkGlError("glGetUniformLocation");
  if (uniId < 0) {
    Log::error("no uniform with name \"%\" found in program", pName);
    return;
  }
  glUniform4fv(uniId, 1, &vec[0]);
  checkGlError("glUniform4fv");
}

//------------------------------------------------------------------------------

void
Program::uniform(const char*    pName,   
                 const Texture& tex,
                 GLint          unit) {
  GLint uniId = glGetUniformLocation(progId_, pName);
  checkGlError("glGetUniformLocation");
  tex.bind(unit);
  if (uniId < 0) {
    Log::error("no uniform with name \"%\" found in program", pName);
    return;
  }

  glUniform1i(uniId, unit);
  checkGlError("glUniform1i (tex)");
}

//------------------------------------------------------------------------------
