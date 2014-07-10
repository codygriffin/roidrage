//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_LINEARINDICATOR_H
#define INCLUDED_LINEARINDICATOR_H

#ifndef INCLUDED_GLM_HPP
#include "glm.hpp"
#define INCLUDED_GLM_HPP
#endif

#ifndef INCLUDED_FUNCTIONAL
#include <functional>
#define INCLUDED_FUNCTIONAL
#endif

#ifndef INCLUDED_RENDERABLE_H
#include "Renderable.h"
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

struct OrthoCamera;

//------------------------------------------------------------------------------

struct LinearIndicator : public Renderable {
  typedef std::function<glm::vec4(float)> Callback;
  LinearIndicator(float x, 
                  float y, 
                  float w = 30.0f, 
                  float h = 4.0f, 
                  float b = 2.0f, 
                  float value = 0.0f);

  void update(float v);
  void color(Callback c);

protected:
  void  onRender(OrthoCamera* pCam);

private:
  std::function<glm::vec4(float)> color_;
  float x_;
  float y_;
  float w_;
  float h_;
  float b_;
  float value_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
