//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_SELECTOR_H
#define INCLUDED_SELECTOR_H

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

struct Selector : public Layout {
  Selector(const std::string& label, 
           std::function<void(Selector&)> callback, 
           int numChoices, 
           int v = 0, float d = 12.0f, float w = 240.0f);
  
  int  getValue() const;
  void callback(std::function<void(Selector&)> c);
  void setLabel(const std::string& label);
  
protected:
  virtual bool onTouch(const Touch& touch); 
  virtual void onRender();
  virtual glm::vec2 onLayout(Layout* pLayout, glm::vec2 trans);

private:
  int   quantizeX(float x);

  std::string label_;
  float       selectorWidth_;
  float       selectorHeight_;
  std::function<void(Selector&)> callback_;

  int numChoices_;
  int value_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
