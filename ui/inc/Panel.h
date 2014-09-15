//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_PANEL_H
#define INCLUDED_PANEL_H

#ifndef INCLUDED_LAYOUT_H
#include <Layout.h>
#endif

//------------------------------------------------------------------------------

namespace ui { 

//------------------------------------------------------------------------------

struct Panel : public roidrage::Layout {
  Panel(const glm::vec4& size, 
        const glm::vec4& color = glm::vec4(0.6f, 0.7f, 0.7f, 0.6f));
  
protected:
  virtual void      onRender();
  virtual void      onRender(glm::mat4& proj, glm::vec2& offset); 
  virtual glm::vec2 onLayout(Layout* pLayout, glm::vec2 trans);
  virtual bool      onEvent(beta::GlfwMouseButton mouse);

private:
  //glm::vec4 size_;
  glm::vec4 color_;
};

//------------------------------------------------------------------------------

} // namespace ui

//------------------------------------------------------------------------------

#endif
