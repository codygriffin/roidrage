//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Shader.h"

#include "Log.h"
#include <memory>
#include <stdexcept>

#define GL_MESSAGES
#include "glm.hpp"
#include "matrix_transform.hpp"
#include "vector_angle.hpp"

// GL
#include "Shader.h"
#include "Program.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "VertexBufferObject.h"
#include "OrthoCamera.h"
#include "Frame.h"

// Structure
#include "Corvid.h"
#include "Relation.h"
#include "Population.h"

// Utils
#include "AssetManager.h"
#include "Log.h"

//------------------------------------------------------------------------------

using namespace pronghorn;

//------------------------------------------------------------------------------

Shader::Shader(GLenum type, const char* pSource) 
  : shaderId_    (0) 
{
  shaderId_ = glCreateShader(type);
  if (!shaderId_) {
    Log::error("Could not create shader.");
    throw;
  }

  glShaderSource(shaderId_, 1, &pSource, 0);
  glCompileShader(shaderId_);

  GLint compiled = GL_FALSE;
  glGetShaderiv(shaderId_, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    GLint bufferLength = 0;
    glGetShaderiv(shaderId_, GL_INFO_LOG_LENGTH, &bufferLength);
    if (bufferLength) {
      std::unique_ptr<GLchar> buffer(new GLchar[bufferLength]);
      glGetShaderInfoLog(shaderId_, bufferLength, 0, buffer.get());
      Log::error("Could not compile shader:\n%\n", buffer.get());
      Log::error("\n%\n", pSource);
    }
    throw;
  }
}

//------------------------------------------------------------------------------

Shader::~Shader() {
  #ifdef GL_CLEANUP
  glDeleteShader(shaderId_);
  #endif
}

//------------------------------------------------------------------------------

GLuint
Shader::getId() const {
  return shaderId_;
}

//------------------------------------------------------------------------------

VertexShader::VertexShader(const char* pSource) 
  : Shader (Shader::VERTEX, pSource) 
{
}

//------------------------------------------------------------------------------

FragmentShader::FragmentShader(const char* pSource) 
  : Shader (Shader::FRAGMENT, pSource) 
{
}

//------------------------------------------------------------------------------
