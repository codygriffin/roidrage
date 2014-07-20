//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_PROGRAM_H
#define INCLUDED_PROGRAM_H

#ifdef ANDROID
#ifndef INCLUDED_GLES_GL2_H
#define INCLUDED_GLES_GL2_H
#include <GLES2/gl2.h>
#endif
#else
#include <GLFW/glfw3.h>
#endif

#ifndef INCLUDED_GLM_HPP
#define INCLUDED_GLM_HPP
#include "glm.hpp"
#endif

#ifndef INCLUDED_MAP
#define INCLUDED_MAP
#include <map>
#endif

#ifndef INCLUDED_MEMORY
#define INCLUDED_MEMORY
#include <memory>
#endif

#ifndef INCLUDED_STRING
#define INCLUDED_STRING
#include <string>
#endif

//------------------------------------------------------------------------------

namespace pronghorn { 

//------------------------------------------------------------------------------

class FragmentShader;
class Texture;
class VertexBufferObject;
class VertexShader;

class Program {
public:
  Program(VertexShader   vertexShader, 
          FragmentShader fragShader,
          const GLenum   primitiveType = defaultPrimitiveType_);
 ~Program();

  GLuint getId() const;

  void   attribute(const char*         pName, 
                   VertexBufferObject& vbo,
                   GLint               size,
                   GLsizei             stride,
                   uintptr_t           offset);


  // Scalar Uniforms
  void   uniform  (const char*  pName,   
                   float        num);

  void   uniform  (const char*  pName,   
                   int          num);

  void   uniform  (const char*         pName,   
                   const glm::mat4&    matrix);

  void   uniform  (const char*         pName,   
                   const glm::vec4&    vec);

  void   uniform  (const char*         pName,   
                   const Texture&      tex,
                   GLint               unit = 0);

  void   use();
  void   execute(GLint offset, GLsizei length);

private:
  const GLenum primitiveType_;

  GLuint progId_;

  //XXX Fake memoization
  static GLuint currentProgId_;
public: 
  static const GLenum triangles     = GL_TRIANGLES;
  static const GLenum triangleFan   = GL_TRIANGLE_FAN;
  static const GLenum triangleStrip = GL_TRIANGLE_STRIP;
  static const GLenum lineLoop      = GL_LINE_LOOP;
  static const GLenum lines         = GL_LINES;

  static const GLenum defaultPrimitiveType_ = triangleFan;
}; 

//------------------------------------------------------------------------------

} // namespace

//------------------------------------------------------------------------------

#endif
