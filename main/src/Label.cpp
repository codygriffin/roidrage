//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include <Label.h>
#include <Display.h>
#include <Systems.h>

//------------------------------------------------------------------------------

using namespace roidrage;

//------------------------------------------------------------------------------

Label::Label(const std::string& text, float d) 
  : text_(text) {
  labelSize_ = d * Display::getScale();
}


//------------------------------------------------------------------------------

void
Label::onRender() {
  float textWidth = labelSize_ * text_.length();
  const float c = 16.0f;
  renderTextOverlay(position_.x - textWidth, 
                    position_.y, 
                    labelSize_, 
                    text_.c_str());
}

//------------------------------------------------------------------------------

glm::vec2 
Label::onLayout(Layout* pParent, glm::vec2 translation) {
  float textWidth = labelSize_ * text_.length();
  dimension_ = glm::vec2(textWidth, labelSize_ * 4);
  return dimension_;
}

//------------------------------------------------------------------------------
