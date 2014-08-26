//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_BETAGAME_H
#define INCLUDED_BETAGAME_H

#ifndef INCLUDED_BETA_H
#include "Beta.h"
#endif

#ifndef INCLUDED_TIMERREACTOR_H
#include "TimerReactor.h"
#endif

#ifndef INCLUDED_LAYOUT_H
#include "Layout.h"
#endif

#ifndef INCLUDED_RANDOM
#include <random>
#define INCLUDED_RANDOM
#endif

//------------------------------------------------------------------------------

namespace beta { 

struct Frame;

//------------------------------------------------------------------------------

struct BetaGame : public BetaMachine::State {
  BetaGame(Beta* pMachine);

protected:
  void onEvent(Tick);
  void onEvent(GlfwKey);
  void onEvent(GlfwMouseButton);
  void onEvent(GlfwMouseMove);
  void onEvent(GlfwMouseScroll);

private:

  // Queue of work to be executed in game-context
  corvid::TimerReactor timerReactor_;
  roidrage::Layout     layout_;
};

//------------------------------------------------------------------------------

} // namespace beta

//------------------------------------------------------------------------------

#endif
