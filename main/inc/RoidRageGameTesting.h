//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_ROIDRAGEGAMETESTING_H
#define INCLUDED_ROIDRAGEGAMETESTING_H

#ifndef INCLUDED_ROIDRAGE_H
#include "RoidRage.h"
#endif

#ifndef INCLUDED_WORKQUEUE_H
#include "WorkQueue.h"
#endif

#ifndef INCLUDED_TIMERREACTOR_H
#include "TimerReactor.h"
#endif

#ifndef INCLUDED_RANDOM
#include <random>
#define INCLUDED_RANDOM
#endif

#include "Entity.h"

//------------------------------------------------------------------------------

namespace roidrage { 

struct Frame;

//------------------------------------------------------------------------------

struct RoidRageGameTesting : public RoidRage::State {
  RoidRageGameTesting(RoidRage* pMachine);

protected:
  void onEvent(Tick);
  void onEvent(AndroidBack);
  void onEvent(GlfwKey);
  void onEvent(GlfwMouseButton);
  void onEvent(GlfwMouseMove);

private:

  // Queue of work to be executed in game-context
  corvid::WorkQueue    gameQueue_;
  corvid::TimerReactor timerReactor_;
  System               game_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
