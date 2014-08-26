//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include <Slider.h>
#include <Display.h>
#include <sstream>

//------------------------------------------------------------------------------

using namespace roidrage;

//------------------------------------------------------------------------------

Slider::Slider(const std::string& label, std::function<void(float)> callback, float v, float d, float w) 
  : label_    (label) 
  , callback_ (callback) 
  , value_    (v) {
  sliderHeight_ = d * Display::getScale();
  sliderWidth_  = w * Display::getScale();
}

//------------------------------------------------------------------------------

void
Slider::callback(std::function<void(float)> callback) {
  callback_ = callback;
}

//------------------------------------------------------------------------------

glm::vec2 
Slider::onLayout(Layout* pParent, glm::vec2 translation) {
  dimension_ = glm::vec2(sliderWidth_, sliderHeight_ * 4);
  return dimension_;
}

//------------------------------------------------------------------------------

bool 
Slider::onTouch(const Touch& touch) {
  const float x = touch.xs[touch.index];
  const float y = touch.ys[touch.index];

  value_ = normalizeX(x);
  callback_(value_);
  return true;
}

//------------------------------------------------------------------------------

void
Slider::onRender() {
  /*
  // LinearIndicator
  const float b = 4.0f; 
  SolidQuad  shield(position_.x, 
                    position_.y + sliderHeight_/2.0f, 
                    sliderWidth_ * value_, 
                    sliderHeight_);

  SolidQuad  shieldBg(position_.x - b, 
                      position_.y  + sliderHeight_/2.0f - b, 
                      sliderWidth_ + (b * 2.0f), 
                      sliderHeight_+ (b * 2.0f));

  // TODO shield color quantization expression
  shield.getRenderPass().color = glm::vec4(0.42f, 0.42f, 0.85, 0.8f);
  shieldBg.getRenderPass().color = glm::vec4(0.07f, 0.07f, 0.07, 0.8f);

  shieldBg.render(RenderState::pCam_.get());
  shield.render(RenderState::pCam_.get());

  renderTextOverlay(position_.x, 
                    position_.y, 
                    sliderHeight_, 
                    label_.c_str());
  */
}

//------------------------------------------------------------------------------

float 
Slider::getValue() const {
  return value_;
}

//------------------------------------------------------------------------------

float 
Slider::normalizeX(float x) {
  return (x - position_.x) / sliderWidth_;
}

//------------------------------------------------------------------------------
