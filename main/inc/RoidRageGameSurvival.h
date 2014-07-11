//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_ROIDRAGEGAMESURVIVAL_H
#define INCLUDED_ROIDRAGEGAMESURVIVAL_H

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

//------------------------------------------------------------------------------

namespace roidrage { 

struct Frame;

//------------------------------------------------------------------------------

struct RoidRageGameSurvival : public RoidRage::State {
  RoidRageGameSurvival(RoidRage* pMachine);

protected:
  void onEvent(Tick);
  void onEvent(Touch);
  void onEvent(AndroidBack);
  void onEvent(GlfwKey);
  void onEvent(GlfwMouseButton);
  void onEvent(GlfwMouseMove);

  void onDown  (int index, int count, int* p, int* x, int* y, int* s);
  void onMove  (int index, int count, int* p, int* x, int* y, int* s);
  void onUp    (int index, int count, int* p, int* x, int* y, int* s);

private:
  bool      isShooting() const;
  glm::vec2 calcThrustVector() const;
  void      spawnTutorialText();
  void      spawnBoids();
  void      spawnRoids();
  
  bool      inverted_;
  float     limit_;
  float     multiplier_;
  int64_t   lastTap_;

  glm::vec2 thrust_;
  glm::vec2 current_;
  glm::vec2 previous_;

  // Queue of work to be executed in game-context
  corvid::WorkQueue    gameQueue_;
  corvid::TimerReactor timerReactor_;

  std::unique_ptr<Frame> pGlow_;

  // TODO Garbage queue?
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
