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

#include "AssetManager.h"
#include "Log.h"

//------------------------------------------------------------------------------

using namespace pronghorn;

//------------------------------------------------------------------------------

Shader::Shader(GLenum type, const char* pSourcePath) 
  : shaderId_   (0) 
  , sourcePath_ (pSourcePath)
{
  auto pSource = AssetManager::loadText(sourcePath_);
  Log::error("Loaded shader from %", pSourcePath);

  shaderId_ = glCreateShader(type);
  if (!shaderId_) {
    Log::error("Could not create shader.");
    throw;
  }

  auto pSourceRaw = pSource.get();
  glShaderSource(shaderId_, 1, &pSourceRaw, 0);
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
      Log::error("\nin %\n", pSourcePath);
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

const std::string&
Shader::sourcePath() const {
  return sourcePath_;
}


//------------------------------------------------------------------------------

VertexShader::VertexShader(const char* pSourcePath) 
  : Shader (Shader::VERTEX, pSourcePath) 
{
}

//------------------------------------------------------------------------------

FragmentShader::FragmentShader(const char* pSourcePath) 
  : Shader (Shader::FRAGMENT, pSourcePath) 
{
}

//------------------------------------------------------------------------------
