//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_LAYOUT_H
#define INCLUDED_LAYOUT_H

#ifndef INCLUDED_GLM_HPP
#include "glm.hpp"
#define INCLUDED_GLM_HPP
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct Touch {
  Touch(int a, int i, int c, int* p, int* x, int* y) {
    action = a;
    index = i;
    count = c;
    xs = x;
    ys = y;
    pointers = p;
  }
   
  int  action;
  int  index;
  int  count;
  int* pointers;
  int* xs;
  int* ys;

  static const int down = 0;
  static const int up   = 1;
  static const int move = 2;
};

struct Layout {
  Layout();
  Layout(unsigned width, unsigned height);
  Layout(glm::vec2 translation, glm::vec2 scale = glm::vec2(1.0f, 1.0f));
  
  void add(Layout* pLayout);
  void clear();

  void render();
  bool touch(const Touch& touch);

  void layout();

  glm::vec2 dimension() const;
  void      dimension(unsigned width, unsigned height);

  glm::vec2 position() const;
  glm::vec2 layout(Layout* pParent, glm::vec2 position);
 
  virtual bool onTouch(const Touch& touch)       { return false; };
  virtual void onRender()                                 {};
  virtual glm::vec2 onLayout(Layout* pParent, glm::vec2 position);

  bool  isHit(float x, float y);

  glm::vec2   translation_;
  glm::vec2   scale_;
  glm::vec2   margin_;

  glm::vec2   dimension_;
  glm::vec2   position_;
  //TODO glm::mat4 transform_;
  
  std::vector<std::unique_ptr<Layout>> children_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
