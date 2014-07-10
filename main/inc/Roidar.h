//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_ROIDAR_H
#define INCLUDED_ROIDAR_H

#ifndef INCLUDED_RENDERABLE_H
#include "Renderable.h"
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct Roidar : public Renderable {
  Roidar();

protected:
  void onRender(OrthoCamera* pCam);
  void preRender(OrthoCamera* pCam);

private:
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
