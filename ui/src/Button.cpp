//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Button.h"
#include "Label.h"
#include "Panel.h"

#include "Log.h"
#include "AssetManager.h"

//------------------------------------------------------------------------------

using namespace roidrage;
using namespace pronghorn;

//------------------------------------------------------------------------------

Button::Button(const std::string& text, Callback callback, float d) 
  : text_     (text) 
  , callback_ (callback) {
  add(new Label(text, d));
}

bool
Button::onEvent(beta::GlfwMouseButton mouse) {
  callback_(mouse);
  return true;
}

void
Button::onRender(glm::mat4& proj, glm::vec2& offset) {
}

glm::vec2 
Button::onLayout(Layout* pParent, glm::vec2 position) {

  // lay out children, stacked
  // we'll fit the background/button to the max
  for (auto child = children_.begin(); child != children_.end(); child++) {
    auto pos = glm::vec2(position_.x, position_.y);
    auto dim = (*child)->layout(this, pos);
    dimension_.x = std::max(dimension_.x, dim.x);
    dimension_.y = std::max(dimension_.y, dim.y);
  }

  add(new ui::Panel(glm::vec4(position_.x, position_.y, dimension_.x, dimension_.y),
                    glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));

  return dimension_;
}

//------------------------------------------------------------------------------
