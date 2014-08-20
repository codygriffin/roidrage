//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include <Display.h>

//------------------------------------------------------------------------------

using namespace roidrage;

float Display::width_  = 0;
float Display::height_ = 0;
float Display::scale_  = 0;

//------------------------------------------------------------------------------

void
Display::reset(float width, float height, float scale) {
  width_  = width;
  height_ = height;
  scale_  = scale;
}

//------------------------------------------------------------------------------

float 
Display::getWidth () {
  return width_;
}

//------------------------------------------------------------------------------

float 
Display::getHeight() {
  return height_;
}

//------------------------------------------------------------------------------

float 
Display::getScale () {
  return scale_;
}

//------------------------------------------------------------------------------

float 
Display::getScaledWidth () {
  return width_ / scale_;
}

//------------------------------------------------------------------------------

float 
Display::getScaledHeight() {
  return height_ / scale_;
}

//------------------------------------------------------------------------------
