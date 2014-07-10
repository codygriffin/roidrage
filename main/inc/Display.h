//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_DISPLAY_H
#define INCLUDED_DISPLAY_H

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct Display  {
  static void reset(float width, float height, float scale);

  static float getWidth () ;
  static float getHeight() ;
  static float getScale () ;

  static float getScaledWidth () ;
  static float getScaledHeight() ;
protected:

private:
  static float width_;
  static float height_;
  static float scale_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
