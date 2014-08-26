//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include <Button.h>
#include <Display.h>

//------------------------------------------------------------------------------

using namespace roidrage;

//------------------------------------------------------------------------------

Button::Button(const std::string& text, std::function<void(void)> callback, float d) 
  : text_(text) 
  , callback_ (callback) {
  labelSize_ = d * Display::getScale();
}

void
Button::callback(std::function<void(void)> callback) {
  callback_ = callback;
}

bool 
Button::onTouch(const Touch& touch) {
  if (touch.action == Touch::up) {
    callback_();
  }

  return true;
}

void
Button::onRender() {
  /*
  renderTextOverlay(position_.x, 
                    position_.y, 
                    labelSize_, 
                    text_.c_str());
  */
}

glm::vec2 
Button::onLayout(Layout* pParent, glm::vec2 position) {
  float textWidth = labelSize_ * text_.length();
  dimension_ = glm::vec2(textWidth, labelSize_ * 4);
  return dimension_;
}

//------------------------------------------------------------------------------
