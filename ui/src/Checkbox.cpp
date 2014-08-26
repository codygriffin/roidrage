//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include <Checkbox.h>
#include <Display.h>
#include <sstream>

//------------------------------------------------------------------------------

using namespace roidrage;

//------------------------------------------------------------------------------

Checkbox::Checkbox(const std::string& label, std::function<void(bool)> callback,bool selection, float d, float w) 
  : label_    (label) 
  , callback_ (callback) 
  , selection_ (selection) {
  checkboxHeight_ = d * Display::getScale();
  checkboxWidth_  = w * Display::getScale();
}

//------------------------------------------------------------------------------

void
Checkbox::callback(std::function<void(bool)> callback) {
  callback_ = callback;
}

//------------------------------------------------------------------------------

glm::vec2 
Checkbox::onLayout(Layout* pParent, glm::vec2 translation) {

  dimension_ = glm::vec2(checkboxWidth_, checkboxHeight_ * 4);
  return dimension_;
}

//------------------------------------------------------------------------------

bool 
Checkbox::onTouch(const Touch& touch) {
  if (touch.action == Touch::down) {
    selection_ = ! selection_;
    callback_(selection_);
  }
  return true;
}

//------------------------------------------------------------------------------

void
Checkbox::onRender() {
  /*
  const float b = 4.0f; 
  SolidQuad  shield(position_.x, 
                    position_.y + checkboxHeight_/2.0f, 
                    checkboxWidth_, 
                    checkboxHeight_);

  SolidQuad  shieldBg(position_.x - b, 
                      position_.y + checkboxHeight_/2.0f - b, 
                      checkboxWidth_ + (b * 2.0f), 
                      checkboxHeight_ + (b * 2.0f));

  // TODO shield color quantization expression
  shield.getRenderPass().color = glm::vec4(0.42f, 0.42f, 0.85, 0.8f);
  shieldBg.getRenderPass().color = glm::vec4(0.07f, 0.07f, 0.07, 0.8f);

  shieldBg.render(RenderState::pCam_.get());
  if (selection_) {
    shield.render(RenderState::pCam_.get());
  }

  renderTextOverlay(position_.x, 
                    position_.y, 
                    checkboxHeight_, 
                    label_.c_str());
  */
}

//------------------------------------------------------------------------------

bool
Checkbox::checked() const {
  return selection_;
}

//------------------------------------------------------------------------------
