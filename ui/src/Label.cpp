//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Label.h"
#include "Display.h"
#include "Geometry.h"
#include "Display.h"
#include "Texture.h"
#include "Program.h"
#include "Shader.h"
#include "VertexBufferObject.h"

#include "Log.h"
#include "AssetManager.h"

#include "matrix_transform.hpp"

//------------------------------------------------------------------------------

using namespace roidrage;
using namespace gl;
using namespace pronghorn;

//------------------------------------------------------------------------------

Label::Label(const std::string& text, float d) 
  : text_(text) {
  labelSize_ = d * Display::getScale();
}


//------------------------------------------------------------------------------

void
Label::onRender(glm::mat4& proj, glm::vec2& offset) {
  unsigned w_, h_;
  static Program            program(VertexShader  ("assets/gpu/char.vp"), 
                                    FragmentShader("assets/gpu/char.fp"));
  static VertexBufferObject vbo  (sizeof(oquad), oquad);
  static Texture            tex  (512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/menlo24.png",    w_, h_).get());

  const float r       = labelSize_;
  const float offsetx = 0.0f;
  const float offsety = 0.0f;
  const float padx    = 0.0f;
  const float pady    = 0.0f;
  const std::string& text = text_;

  const glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

  offset = glm::vec2();
  auto model = glm::translate(glm::mat4(), glm::vec3(position_.x, 
                                                   -(position_.y + dimension_.y),
                                                     0.0f));
  auto matProjection = proj;
  auto matView       = model;

  program.use();
  program.uniform("mOrtho",      matProjection);
  program.uniform("uTexture",    tex);
  program.uniform("vColor",      color);
  program.attribute("vPosition", vbo, 2, 4*sizeof(float), 0);
  program.attribute("vTexture",  vbo, 2, 4*sizeof(float), 2*sizeof(float));

  float x_ = 0; 
  float y_ = 0;
  for (size_t i  = 0; i < text.length(); i++) {
    if (text[i] == '\n') {
      y_ += r + pady; 
      x_ = 0;
    } else {
      int nx = text[i] % 16;
      int ny = text[i] / 16;

      // character offset within texture
      auto translated = glm::translate(matView, glm::vec3(x_, y_, 0.0f));
      auto scaled     = glm::scale(translated, glm::vec3(r));

      program.uniform  ("offsetX", (float)nx);
      program.uniform  ("offsetY", (float)ny);
      program.uniform  ("mModel", scaled);

      program.execute  (0, 4);

      x_ += r * 0.4f;
    }
  }
}

//------------------------------------------------------------------------------

glm::vec2 
Label::onLayout(Layout* pParent, glm::vec2 position) {
  float textWidth = labelSize_ * text_.length();
  dimension_ = glm::vec2(textWidth, labelSize_);
  return dimension_;
}

//------------------------------------------------------------------------------
