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
  typedef std::function<void(beta::GlfwMouseButton mouse)> Callback;
  Button(const std::string& text, Callback callback, float f = 12.0f);

protected:
  virtual void onRender(glm::mat4& proj, glm::vec2& offset);


  virtual glm::vec2 onLayout(Layout* pLayout, glm::vec2 trans);
  virtual bool      onEvent(beta::GlfwMouseButton mouse);
  
private:
  Callback callback_;

  std::string               text_;
  float                     labelSize_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
