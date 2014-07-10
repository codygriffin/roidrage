//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include <LinearIndicator.h>

//------------------------------------------------------------------------------

using namespace roidrage;

//------------------------------------------------------------------------------

LinearIndicator::LinearIndicator(float x, float y, float w, float h, float b, float value) 
  : x_ (x)
  , y_ (y)
  , w_ (w)
  , h_ (h)
  , b_ (b)
  , value_ (value)
{
}

void 
LinearIndicator::color(Callback c) {
  color_ = c;
}

void 
LinearIndicator::update(float v) {
  value_ = v;
}

void  
LinearIndicator::onRender(OrthoCamera* pCam) {
  SolidQuad  shield(x_, 
                    y_, 
                   (w_ * value_) * 2.0f, 
                    h_           * 2.0f);

  SolidQuad  shieldBg(x_ - b_, 
                      y_ - b_, 
                     (w_ + b_) * 2.0f, 
                     (h_ + b_) * 2.0f);

  if (color_) {
    shield.getRenderPass().color = color_(value_);
  } else {
    shield.getRenderPass().color = glm::vec4(0.75f, 0.12f, 0.12, 1.0f);
  }

  shieldBg.getRenderPass().color = glm::vec4(0.07f, 0.07f, 0.07, 0.5f);

  shieldBg.render(pCam);
  shield.render(pCam);
}

//------------------------------------------------------------------------------
