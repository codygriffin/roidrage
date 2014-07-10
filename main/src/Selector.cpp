//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include <Selector.h>
#include <Display.h>
#include <Systems.h>
#include <sstream>

//------------------------------------------------------------------------------

using namespace roidrage;

//------------------------------------------------------------------------------

Selector::Selector(const std::string& label, std::function<void(Selector&)> callback, int numChoices, int v, float d, float w) 
  : label_      (label) 
  , callback_   (callback) 
  , numChoices_ (numChoices)
  , value_      (v) {
  selectorHeight_ = d * Display::getScale();
  selectorWidth_  = w * Display::getScale();
}

//------------------------------------------------------------------------------

void
Selector::callback(std::function<void(Selector&)> callback) {
  callback_ = callback;
}

//------------------------------------------------------------------------------

void 
Selector::setLabel(const std::string& label) {
  label_ = label;
}

//------------------------------------------------------------------------------

glm::vec2 
Selector::onLayout(Layout* pParent, glm::vec2 translation) {
  float textWidth   = selectorHeight_ * label_.length();
  float selectorWidth = selectorHeight_ * 20;

  dimension_ = glm::vec2(textWidth + selectorWidth, selectorHeight_ * 4);
  return dimension_;
}

//------------------------------------------------------------------------------

bool 
Selector::onTouch(const Touch& touch) {
  const float x = touch.xs[touch.index];
  const float y = touch.ys[touch.index];

  value_ = quantizeX(x);
  callback_(*this);
  return true;
}

//------------------------------------------------------------------------------

void
Selector::onRender() {
  float v = (float)value_ / ((float)numChoices_ - 1);
  // LinearIndicator
  const float b = 4.0f; 
  SolidQuad  shield(position_.x, 
                    position_.y + selectorHeight_/2.0f, 
                    selectorWidth_ * v, 
                    selectorHeight_);

  SolidQuad  shieldBg(position_.x - b, 
                      position_.y  + selectorHeight_/2.0f - b, 
                      selectorWidth_ + (b * 2.0f), 
                      selectorHeight_+ (b * 2.0f));

  // TODO shield color quantization expression
  shield.getRenderPass().color = glm::vec4(0.42f, 0.42f, 0.85, 0.8f);
  shieldBg.getRenderPass().color = glm::vec4(0.07f, 0.07f, 0.07, 0.8f);

  shieldBg.render(RenderState::pCam_.get());
  shield.render(RenderState::pCam_.get());

  renderTextOverlay(position_.x, 
                    position_.y, 
                    selectorHeight_, 
                    label_.c_str());
}

//------------------------------------------------------------------------------

int 
Selector::getValue() const {
  return value_;
}

//------------------------------------------------------------------------------

int 
Selector::quantizeX(float x) {
  float n = (x - position_.x) / selectorWidth_;
  int   choice = floor(n * (float)(numChoices_));
  return std::min(choice, numChoices_ - 1);
}

//------------------------------------------------------------------------------
