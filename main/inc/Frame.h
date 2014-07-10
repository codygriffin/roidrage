//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_FRAME_H
#define INCLUDED_FRAME_H

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

#ifndef INCLUDED_RENDERABLE_H
#include "Renderable.h"
#endif

//------------------------------------------------------------------------------

namespace pronghorn { 
  struct Texture;
  struct Framebuffer;
}

namespace roidrage { 

//------------------------------------------------------------------------------

struct Frame : public Quad  {
  Frame(int x, int y);

  std::unique_ptr<pronghorn::Texture>     pFrameTexture_;
  std::unique_ptr<pronghorn::Framebuffer> pFbo_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
