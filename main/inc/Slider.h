//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_SLIDER_H
#define INCLUDED_SLIDER_H

#ifndef INCLUDED_STRING
#include <string>
#define INCLUDED_STRING
#endif

#ifndef INCLUDED_FUNCTIONAL
#include <functional>
#define INCLUDED_FUNCTIONAL
#endif

#ifndef INCLUDED_LAYOUT_H
#include <Layout.h>
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct Slider : public Layout {
  Slider(const std::string& label, std::function<void(float)> callback, float v = 0.0f, float d = 12.0f, float w = 240.0f);
  
  float getValue() const;
  void callback(std::function<void(float)> c);
  
protected:
  virtual bool onTouch(const Touch& touch); 
  virtual void onRender();
  virtual glm::vec2 onLayout(Layout* pLayout, glm::vec2 trans);

private:
  bool  isHit(float x, float y);
  float normalizeX(float x);

  std::string label_;
  float       sliderHeight_;
  float       sliderWidth_;
  std::function<void(float)> callback_;

  float value_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
