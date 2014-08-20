//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_SHADER_H
#define INCLUDED_SHADER_H

#ifdef ANDROID
#ifndef INCLUDED_GLES_GL2_H
#define INCLUDED_GLES_GL2_H
#include <GLES2/gl2.h>
#endif
#else
#include <GLFW/glfw3.h>
#endif

#include <string>

//------------------------------------------------------------------------------

namespace pronghorn {

//------------------------------------------------------------------------------

class Shader {
public:
  Shader(GLenum type, const char* pSource);
 ~Shader();

  GLuint getId() const;
  const std::string& sourcePath() const;

private:
  GLuint shaderId_;
  std::string sourcePath_;

public:
  static const GLenum VERTEX    = GL_VERTEX_SHADER;
  static const GLenum FRAGMENT  = GL_FRAGMENT_SHADER;
}; 

//------------------------------------------------------------------------------

class VertexShader : public Shader {
public:
  VertexShader(const char* pSource);
};

//------------------------------------------------------------------------------

class FragmentShader : public Shader {
public:
  FragmentShader(const char* pSource);
};

//------------------------------------------------------------------------------

} // namespace

//------------------------------------------------------------------------------

#endif
