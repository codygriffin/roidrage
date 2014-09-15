//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Panel.h"
#include "Geometry.h"
#include "Display.h"
#include "Program.h"
#include "Shader.h"
#include "VertexBufferObject.h"

#include "Log.h"

#include "matrix_transform.hpp"

//------------------------------------------------------------------------------

using namespace ui;
using namespace gl;
using namespace pronghorn;

//------------------------------------------------------------------------------

Panel::Panel(const glm::vec4& size, const glm::vec4& color) 
  : Layout(size.x, size.y, size.z, size.w)
  , color_(color) { 
}

//------------------------------------------------------------------------------

glm::vec2 
Panel::onLayout(Layout* pParent, glm::vec2 offset) {
  Log::debug("Panel::onLayout pos=(%,%) dim=(%,%)", position_.x, position_.y, dimension_.x, dimension_.y);

  // lay out children, vertically XXX
  float y = 0.0f;
  for (auto child = children_.begin(); child != children_.end(); child++) {
    auto pos = glm::vec2(position_.x + 0.0f, position_.y + y);
    auto dim = (*child)->layout(this, pos);
    y += dim.y;
  }

  return dimension_;
}

//------------------------------------------------------------------------------

bool
Panel::onEvent(beta::GlfwMouseButton mouse) { 
  return false; 
};

void
Panel::onRender() {
}

void
Panel::onRender(glm::mat4& proj, glm::vec2& offset) {
  // All Panels use the same shaders/geometry
  static Program            solid(VertexShader  ("assets/gpu/transform.vp"), 
                                  FragmentShader("assets/gpu/solid.fp"));
  static VertexBufferObject vbo  (sizeof(oquad), oquad);
  
  offset = glm::vec2();
  auto model = glm::translate(glm::mat4(), glm::vec3(position_.x, 
                                                   -(position_.y + dimension_.y),
                                                     0.0f));

  // We don't want to scale to propagate - only translation
  model = glm::scale(model, glm::vec3(dimension_.x, dimension_.y, 1.0f));

  // run our GLSL program
  solid.use();
  solid.uniform  ("mOrtho",    proj);
  solid.uniform  ("mModel",    model);
  solid.uniform  ("vColor",    color_);
  solid.attribute("vPosition", vbo, 2, 4*sizeof(float), 0);
  solid.attribute("vTexture",  vbo, 2, 4*sizeof(float), 2*sizeof(float));
  solid.execute  (0, 4);
}

//------------------------------------------------------------------------------
