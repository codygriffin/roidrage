//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_CHECKBOX_H
#define INCLUDED_CHECKBOX_H

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

struct Checkbox : public Layout {
  Checkbox(const std::string& label, 
           std::function<void(bool)> callback, 
           bool selection, float d = 12.0f, float w = 240.0f);
  
  bool checked() const;
  void callback(std::function<void(bool)> c);
  
protected:
  virtual bool onTouch(const Touch& touch); 
  virtual void onRender();
  virtual glm::vec2 onLayout(Layout* pLayout, glm::vec2 trans);

private:
  int   quantizeX(float x);

  std::string label_;
  float       checkboxHeight_;
  float       checkboxWidth_;
  std::function<void(bool)> callback_;

  bool selection_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
