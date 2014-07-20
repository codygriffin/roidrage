//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_ORTHOCAMERA_H
#define INCLUDED_ORTHOCAMERA_H

#ifndef INCLUDED_GLM_HPP
#include "glm.hpp"
#define INCLUDED_GLM_HPP
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct OrthoCamera  {
  OrthoCamera(float width, float height);

  void setPosition(const glm::vec2& pos);
  void setZoom    (float zoom);
  void setTilt    (float tilt);

  const glm::mat4& getOrthoMatrix()   const;
  const glm::mat4& getOverlayMatrix() const;
  float            getZoom         () const;

  float            getWidth () const;
  float            getHeight() const;

  glm::vec2        toWorld(const glm::vec2& p) {
    return glm::vec2(p.x - width_/2.0f, p.y - height_/2.0) + pos_;
  }

  glm::vec2        toScreen(const glm::vec2& p) {
    return glm::vec2(p.x + width_/2.0f, p.y + height_/2.0) - pos_;
  }
  
protected:

private:
  float     width_;
  float     height_;

  float     zoom_;
  float     tilt_;
  glm::vec2 pos_;

  // Cached matrix
  mutable glm::mat4 ortho_;
  mutable bool      dirty_;

  mutable glm::mat4 overlay_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
