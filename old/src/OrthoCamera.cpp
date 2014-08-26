//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "OrthoCamera.h"
#include "Display.h"

#include "matrix_transform.hpp"

using namespace roidrage;

//------------------------------------------------------------------------------
// TODO resolution independence?

OrthoCamera::OrthoCamera(float width, float height) 
  : width_ (width) 
  , height_(height)
  , zoom_  (0.0f) 
  , tilt_  (0.0f) 
  , dirty_ (true) {

  setZoom (0.15f); 
  overlay_ = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
}

//------------------------------------------------------------------------------

void 
OrthoCamera::setPosition(const glm::vec2& pos) {
  pos_ = pos;
  dirty_ = true;
}

//------------------------------------------------------------------------------

void 
OrthoCamera::setZoom(float zoom) {
  zoom_ = zoom * Display::getScale();
  dirty_ = true;
}

//------------------------------------------------------------------------------

void 
OrthoCamera::setTilt(float tilt) {
  tilt_ = tilt;
  dirty_ = true;
}

//------------------------------------------------------------------------------

const glm::mat4& 
OrthoCamera::getOrthoMatrix() const {
  // memoization
  if (dirty_) {
    ortho_= glm::ortho(pos_.x - ((width_/2.0f)  / zoom_), 
                       pos_.x + ((width_/2.0f)  / zoom_), 
                       pos_.y + ((height_/2.0f) / zoom_), 
                       pos_.y - ((height_/2.0f) / zoom_), 
                       -1.0f, 1.0f);
    dirty_ = false;
  }

  return ortho_;
}

//------------------------------------------------------------------------------

const glm::mat4& 
OrthoCamera::getOverlayMatrix() const {
  return overlay_;
}

//------------------------------------------------------------------------------

float
OrthoCamera::getWidth() const {
  return width_;
}

//------------------------------------------------------------------------------

float
OrthoCamera::getHeight() const {
  return height_;
}

//------------------------------------------------------------------------------

float
OrthoCamera::getZoom() const {
  return zoom_;
}

//------------------------------------------------------------------------------
