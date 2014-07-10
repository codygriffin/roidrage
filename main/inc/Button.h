//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_BUTTON_H
#define INCLUDED_BUTTON_H

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

struct Button : public Layout {
  Button(const std::string& text, std::function<void(void)> callback, float d = 12.0f);
  
  void callback(std::function<void(void)> c);

protected:
  virtual bool onTouch(const Touch& touch); 
  virtual void onRender();

  virtual glm::vec2 onLayout(Layout* pLayout, glm::vec2 trans);
  
private:
  std::function<void(void)> callback_;

  std::string text_;
  float labelSize_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
