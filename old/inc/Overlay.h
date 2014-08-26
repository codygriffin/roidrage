//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_OVERLAY_H
#define INCLUDED_OVERLAY_H

#ifndef INCLUDED_RENDERABLE_H
#include "Renderable.h"
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct OrthoCamera;

struct Overlay : public Renderable {
  Overlay();

protected:
  virtual void preRender(OrthoCamera* pCam);
  virtual void  onRender(OrthoCamera* pCam);
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
