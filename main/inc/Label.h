//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_LABEL_H
#define INCLUDED_LABEL_H

#ifndef INCLUDED_STRING
#include <string>
#define INCLUDED_STRING
#endif

#ifndef INCLUDED_LAYOUT_H
#include <Layout.h>
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct Label : public Layout {
  Label(const std::string& text, float d = 12.0f);

protected:
  virtual void onRender();
  virtual glm::vec2 onLayout(Layout* pLayout, glm::vec2 trans);
  
private:
  std::string text_;
  float labelSize_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
