//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_ROIDRAGESCORE_H
#define INCLUDED_ROIDRAGESCORE_H

#ifndef INCLUDED_ROIDRAGE_H
#include "RoidRage.h"
#endif

#ifndef INCLUDED_LAYOUT_H
#include "Layout.h"
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct RoidRageScore : public RoidRage::State {
  RoidRageScore(RoidRage* pMachine);

  void onEvent(Tick);
  void onEvent(Touch);
  void onEvent(AndroidBack);
protected:

private:
  unsigned first_;
  unsigned page_;
  unsigned max_;

  Layout scoreLayout_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
