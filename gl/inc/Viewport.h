//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_VIEWPORT_H
#define INCLUDED_VIEWPORT_H

#ifndef INCLUDED_GL_H
#include "Gl.h"
#endif

#ifndef INCLUDED_GLM_HPP
#include "glm.hpp"
#define INCLUDED_GLM_HPP
#endif

//------------------------------------------------------------------------------

namespace gl {

//------------------------------------------------------------------------------

class Viewport {
public:
  static
  void reset(const glm::vec4& viewport) {
    viewport_ = viewport;
    glViewport(viewport_.x, viewport_.y, viewport_.z, viewport_.w);
  }

  static
  const glm::vec4& get() {
    return viewport_;
  }

private:
  static glm::vec4 viewport_;
}; 

//------------------------------------------------------------------------------

} // namespace

//------------------------------------------------------------------------------

#endif
