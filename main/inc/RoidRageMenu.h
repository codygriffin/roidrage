//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_ROIDRAGEMENU_H
#define INCLUDED_ROIDRAGEMENU_H

#ifndef INCLUDED_ROIDRAGE_H
#include "RoidRage.h"
#endif

#ifndef INCLUDED_BUTTON_H
#include "Button.h"
#endif

#ifndef INCLUDED_LABEL_H
#include "Label.h"
#endif

#include <string>

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct RoidRageScore;
struct RoidRageSettings;
struct RoidRageGame;
struct RoidRageGameSurvival;
struct ConfirmNewGame;
struct ConfirmQuit;

struct RoidRageMenu : public RoidRage::State {
  RoidRageMenu(RoidRage* pMachine, bool populateRoids);

protected:
  void onEvent(Tick);
  void onEvent(Touch);
  void onEvent(AndroidBack);

private:
  Layout menuLayout_;
};

//------------------------------------------------------------------------------

struct ConfirmNewGame : public RoidRageMenu {
  ConfirmNewGame(RoidRage* pMachine);

protected:
  void onEvent(Tick);
  void onEvent(Touch);
  void onEvent(AndroidBack);

private:
  Layout menuLayout_;
};

//------------------------------------------------------------------------------

struct Terminate : public RoidRage::State {
  Terminate(RoidRage *pMachine) 
  : RoidRage::State(pMachine) {
    exit(0);
  }
};

//------------------------------------------------------------------------------

struct ConfirmQuit : public RoidRageMenu {
  ConfirmQuit(RoidRage* pMachine);

protected:
  void onEvent(Tick);
  void onEvent(Touch);
  void onEvent(AndroidBack);

private:
  Layout menuLayout_;
};

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
