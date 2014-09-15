//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include <Layout.h>
#include <Log.h>

using namespace pronghorn;
using namespace roidrage;

//------------------------------------------------------------------------------

Layout::Layout() {
}

Layout::Layout(float x, float y, float w, float h) {
  position_  = glm::vec2(x, y);
  dimension_ = glm::vec2(w, h); 
}

void
Layout::layout() {
  layout(0, glm::vec2());
}

glm::vec2
Layout::layout(Layout* pParent, glm::vec2 position) {
  // Take care of our own layout (within the context of the parent)
  position_ += position;
  return onLayout(pParent, position);
}

glm::vec2
Layout::onLayout(Layout* pParent, glm::vec2 trans) {
  Log::info("Layout::onLayout pos=(%,%) dim=(%,%)", position_.x, position_.y, dimension_.x, dimension_.y);

  // lay out children, vertically
  float y = 0.0f;
  for (auto child = children_.begin(); child != children_.end(); child++) {
    auto pos = glm::vec2(position_.x + 0.0f, position_.y + y);
    auto dim = (*child)->layout(this, pos);
    y += dim.y;
  }

  return dimension_;
}


void 
Layout::add(Layout* pLayout) {
  children_.push_back(std::unique_ptr<Layout>(pLayout));
}

void 
Layout::clear() {
  children_.clear();
}

void 
Layout::render() {
  onRender(); // Render before children

  for (auto child = children_.begin(); child != children_.end(); child++) {
    (*child)->render();
  }
}

void 
Layout::render(glm::mat4 proj, glm::vec2 offset) {
  onRender(proj, offset); // Render before children

  for (auto child = children_.begin(); child != children_.end(); child++) {
    (*child)->render(proj, offset);
  }
}

bool 
Layout::touch(const Touch& touch) {
  const float x = touch.xs[touch.index];
  const float y = touch.ys[touch.index];

  // We handle it ourselves
  if (onTouch(touch)) return true;

  // We delegate to our children
  for (auto child = children_.begin(); child != children_.end(); child++) {
    if ((*child)->isHit(x, y)) {
      if ((*child)->touch(touch)) 
        return true;
    }
  }

  // We dropped the ball...
  return false;
}

// Forwar event to children
bool 
Layout::event(beta::GlfwMouseButton mouse) {
  // We handle it ourselves
  if (onEvent(mouse)) return true;

  // We delegate to our children (that hit)
  for (auto child = children_.begin(); child != children_.end(); child++) {
    if ((*child)->isHit(mouse.x, mouse.y)) {
      if ((*child)->event(mouse)) 
        return true;
    }
  }

  // We dropped the ball...
  return false;
}

bool
Layout::onEvent(beta::GlfwMouseButton mouse) { 
  return false; 
};

bool 
Layout::isHit(float x, float y) {
  return x >= position_.x && x <= position_.x+dimension_.x &&
         y >= position_.y && y <= position_.y+dimension_.y;
}

//------------------------------------------------------------------------------

glm::vec2 
Layout::dimension() const {
  return dimension_;
}

//------------------------------------------------------------------------------

glm::vec2 
Layout::position() const {
  return position_;
}

//------------------------------------------------------------------------------
