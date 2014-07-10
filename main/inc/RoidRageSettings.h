//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_ROIDRAGESETTINGS_H
#define INCLUDED_ROIDRAGESETTINGS_H

#ifndef INCLUDED_ROIDRAGE_H
#include "RoidRage.h"
#endif

#ifndef INCLUDED_LAYOUT_H
#include "Layout.h"
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct RoidRageSettings : public RoidRage::State {
  RoidRageSettings(RoidRage* pMachine);

  void onEvent(Tick);
  void onEvent(Touch);
  void onEvent(AndroidBack);
protected:

private:
  void save();
  void load();

  Layout settingsLayout_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
