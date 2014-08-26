//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_BACKGROUND_H
#define INCLUDED_BACKGROUND_H

#ifndef INCLUDED_RENDERABLE_H
#include "Renderable.h"
#endif

#ifndef INCLUDED_PARAMS_H
#include "Params.h"
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

template <typename T>
struct Tween;

template <>
struct Tween<void> {
};

template <typename T>
struct Tween : Tween<void> {
};

struct Tweenable {
  //std::vector<Tween<void>*> tweens_;
};

struct Background : public Quad 
                  , public Tweenable
                  , public TimeDelta
                  , public TimeMs
{
  Background();
  virtual ~Background(); 

protected:
  void preRender(OrthoCamera* pCam);

private:
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
