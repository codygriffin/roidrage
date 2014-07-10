//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_RENDERSTATE_H
#define INCLUDED_RENDERSTATE_H

#ifndef INCLUDED_MEMORY
#define INCLUDED_MEMORY
#include <memory>
#endif


//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------
struct OrthoCamera;
struct RenderState  {
  static void reset(int width, int height);

  static std::unique_ptr<OrthoCamera> pCam_;
protected:

private:
  
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
