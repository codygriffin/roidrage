//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_ROIDRAGEGAMEOVER_H
#define INCLUDED_ROIDRAGEGAMEOVER_H

#ifndef INCLUDED_ROIDRAGE_H
#include "RoidRage.h"
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct RoidRageGameOver : public RoidRage::State {
  RoidRageGameOver(RoidRage* pMachine, bool survival = false);

protected:
  void onEvent(Tick);
  void onEvent(Touch);
  void onEvent(AndroidBack);

private:
  bool      survival_;
  unsigned  secs_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
