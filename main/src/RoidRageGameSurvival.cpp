//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "RoidRageGameSurvival.h"
#include "RoidRageMenu.h"
#include "RoidRageGameOver.h"

#include "Shader.h"
#include "Program.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "VertexBufferObject.h"
#include "OrthoCamera.h"
#include "Frame.h"

#include "Systems.h"
#include "Params.h"
#include "RenderState.h"
#include "Display.h"

#include "Renderable.h"
#include "Ship.h"
#include "Boid.h"
#include "Roid.h"
#include "Roidar.h"
#include "Background.h"
#include "Overlay.h"
#include "GestureTracker.h"

#include "Log.h"
#include "AudioState.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

#define ROIDS
#define BOIDS
#define TUTORIAL
#define THRUST_MULTIPLIER

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace roidrage;
using namespace corvid;

//------------------------------------------------------------------------------

RoidRageGameSurvival::RoidRageGameSurvival(RoidRage* pMachine) 
  : RoidRage::State(pMachine) 
  , inverted_   (true) 
  , multiplier_ (1.0f)
  , lastTap_    (0.0f)
{ 
  pRoidRage->tutorial.push("survival");
  pRoidRage->tutorial.push("they'll never stop...");
  pRoidRage->tutorial.push("good luck!");

  pRoidRage->lives    = 1; 
  pRoidRage->stage    = 0; 
  pRoidRage->gameTime = 0; 
  pRoidRage->score    = 0; 

  pRoidRage->population.clear();

  Log::info("Populating survival game...");

  if (!pRoidRage->pShip) {
    pRoidRage->pShip = new Ship();
    pRoidRage->population.add(new Background());
    pRoidRage->population.add(pRoidRage->pShip);
  }

  Log::info("Population initialized for RoidRageGameSurvival");

  #ifdef BOIDS
  timerReactor_.setPeriodic(std::chrono::seconds(6), [&] () {
    gameQueue_.enqueue( [&] () {
      spawnBoids();
    });
  });
  #endif

  #ifdef ROIDS
  timerReactor_.setPeriodic(std::chrono::seconds(3), [&] () {
    gameQueue_.enqueue( [&] () {
      spawnRoids();
    });
  });
  #endif

  #ifdef TUTORIAL
  timerReactor_.setPeriodic(std::chrono::seconds(5), [&] () {
    gameQueue_.enqueue( [&] () {
      spawnTutorialText();
    });
  });
  #endif

  //pRoidRage->pMusicPlayer->loop(pRoidRage->music.get(), 3351259);
}

//------------------------------------------------------------------------------
#define DUMP_VEC2(n,v) Log::debug(#n "(%, %)", v.x, v.y);

// XXX Ugly MACRO for removing members of the population
#define REMOVE(x) for (auto it : pRoidRage->population.getRelation<x*>().relation_) { \
    x* p##x = std::get<0>(it); \
    if (!p##x->isValid()) { \
      pRoidRage->population.remove(p##x); \
    } \
  } \

void 
RoidRageGameSurvival::onEvent(Tick tick) {
  // TODO Collect garbage - RAII?
  REMOVE(Ship);
  REMOVE(Boid);
  REMOVE(PewPew);
  REMOVE(Roid);
  REMOVE(Bonus);
  REMOVE(Text);

  pRoidRage->population.visit(&updateTime);
  pRoidRage->population.visit(&resetAcceleration);

  pRoidRage->population.pair(&updateAcceleration);

  Boid::reset();
  pRoidRage->population.visit(&updateBoidAverages);
  Boid::calcAverage();

  pRoidRage->population.visit(&updateBoidPosition);
  pRoidRage->population.pair (&updateBoidAvoidance);

  pRoidRage->population.pair (&updateCollision<Ship, Bonus>);
  pRoidRage->population.pair (&updateCollision<Roid, Ship>);
  pRoidRage->population.pair (&updateCollision<Roid, PewPew>);
  pRoidRage->population.pair (&updateCollision<Boid, PewPew>);
  pRoidRage->population.pair (&updateCollision<Ship, PewPew>);

  // TODO: collisions w/ the background?
  pRoidRage->population.visit(&enforceBoundaries);

  pRoidRage->population.visit(&updateThrust);
  pRoidRage->population.visit(&updatePosition);
  pRoidRage->population.visit(&updateTransform); 


  // Orthographic projection, centered on our ship
  // TODO this seems like a clumsy way to track the ship
  RenderState::pCam_->setPosition(pRoidRage->pShip->pos);
  pRoidRage->ortho = RenderState::pCam_->getOrthoMatrix(); 

  if (pRoidRage->sexy) {
    if (!pGlow_.get()) {
      pGlow_.reset(new Frame(Display::getWidth(), Display::getHeight()));
    }

    pGlow_->pFbo_->bind();
    SolidQuad bg(0.0f, 0.0f, 
                 Display::getWidth(), Display::getHeight());
    bg.getRenderPass().color = glm::vec4(0.0f, 0.0f, 0.0f, 0.15f);
    renderOrtho(&bg);
    pRoidRage->population.visit(&renderGlow); 
    Framebuffer::unbind();
  }

  glClearColor(0.07f, 0.07f, 0.13f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  pRoidRage->population.visit(&renderOrtho); 
  
  if (pRoidRage->sexy) {
    renderFrame(&pGlow_->getRenderPass(), Display::getWidth(), Display::getHeight(), 0.5f);
  }

  // Sets own ortho view
  // TODO Now we get a projection matrix that matches screen coordinates
  pRoidRage->ortho = RenderState::pCam_->getOverlayMatrix();
  Overlay overlay;
  renderOverlay(&overlay);

  // TODO this is a garbage collection task - RAII?
  // Cleanup expired/deleted items
  pRoidRage->population.visit(&age);
  pRoidRage->population.visit(&expirePewPews);
  pRoidRage->population.visit(&expiredBonuses);
  pRoidRage->population.visit(&expiredText);
  pRoidRage->population.visit(&expirePowerups);

  // Regenerate shields
  pRoidRage->population.visit(&regenerateShield);

  // TODO: Better idea of gameTime (superclass?)
  pRoidRage->gameTime += pRoidRage->pShip->ms;

  while (gameQueue_.work());

  // If we have no more lives, game over
  if (pRoidRage->population.getRelation<Ship*>().count() == 0) {
    pRoidRage->transition<RoidRageGameOver>(true);
  }
}

//------------------------------------------------------------------------------

void 
RoidRageGameSurvival::spawnTutorialText() {
  if (pRoidRage->tutorial.empty()) {
    return;
  }

  Log::info("Tuturial: %", pRoidRage->tutorial.front());
  Text* pText = new Text(pRoidRage->pShip->pos.x-400.0f, pRoidRage->pShip->pos.y-100.0f);
  pText->expiresInMs = 5000;
  pText->setText("%", pRoidRage->tutorial.front());
  pRoidRage->population.add(pText);
  pRoidRage->tutorial.pop();
}

//------------------------------------------------------------------------------

void 
RoidRageGameSurvival::spawnBoids() {
  if (pRoidRage->population.getRelation<Boid*>().count() > 20) {
    return;
  }

  glm::vec4 pos(6000.0f, 6000.0f, 0.0f, 0.0f);
  pos = glm::rotate(glm::mat4(), (float)(rand() % 3600)/10.0f, 
    glm::vec3(0.0f, 0.0f, 1.0f)) * pos;

  unsigned gameSecs = pRoidRage->gameTime/1000;
  unsigned numBoids = std::max((unsigned)1, rand()%((gameSecs / 20)+1));
  Log::info("Spawning % boids!", numBoids);

  // Add our boids
  while (numBoids > 0) {
    Boid* pBoid = new Boid(pos.x + (numBoids*50.0f), pos.y + (numBoids*50.0f));  
    pRoidRage->population.add(pBoid);
    numBoids--;
  }
}

void 
RoidRageGameSurvival::spawnRoids() {
  if (pRoidRage->population.getRelation<Roid*>().count() > 100) {
    return;
  }

  unsigned gameSecs = pRoidRage->gameTime/1000;
  unsigned numRoids = std::max((unsigned)1, rand()%((gameSecs / 15)+1));
  Log::info("Spawning % roids!", numRoids);

  while (numRoids > 0) {
    glm::vec4 pos(6000.0f, 6000.0f, 0.0f, 0.0f);
    pos = glm::rotate(glm::mat4(), (float)(rand() % 3600)/10.0f, 
      glm::vec3(0.0f, 0.0f, 1.0f)) * pos;
  
    Roid* pRoid = new Roid(pos.x + (numRoids*50.0f), pos.y + (numRoids*50.0f));
    pRoid->Position::vel = -glm::normalize(pRoid->Position::pos) * 0.3f;

    pRoidRage->population.add(pRoid);
    numRoids--;
  }
}

//------------------------------------------------------------------------------

void 
RoidRageGameSurvival::onEvent(Touch touch) {
  const float x = touch.xs[touch.index];
  const float y = touch.ys[touch.index];

  current_ = glm::vec2(x - Display::getWidth()  * 0.5f, 
                       y - Display::getHeight() * 0.5f);

  //static GestureStream gestureStream;
  //gestureStream.onTouch(touch);

  switch (touch.action) {
  case Touch::down:
    onDown(touch.index, touch.count, touch.pointers, touch.xs, touch.ys, 0);
    break;
  case Touch::up:
    onUp(touch.index, touch.count, touch.pointers, touch.xs, touch.ys, 0);
    break;
  case Touch::move:
    onMove(touch.index, touch.count, touch.pointers, touch.xs, touch.ys, 0);
    break;
  }

  previous_ = current_;
}

//------------------------------------------------------------------------------

void 
RoidRageGameSurvival::onEvent(GlfwKey key) {
  pRoidRage->pShip->shoot();
}


void 
RoidRageGameSurvival::onEvent(GlfwMouseMove mouse) {
}

void 
RoidRageGameSurvival::onEvent(GlfwMouseButton mouse) {
}

void 
RoidRageGameSurvival::onDown(int index, int count, int* p, int* x, int* y, int* s) {
  //if (isShooting()) {
  //  pRoidRage->pShip->shoot();
  //  return;
  // }

  if (count == 1) {
    #ifdef THRUST_MULTIPLIER
    const float tapMs = 250.0f;
    int64_t now = TimeMs::now(); 
    if (now - lastTap_ < tapMs && glm::length(previous_ - current_) < 100.0f) {
      multiplier_ = 3.0f;
    } else {
      multiplier_ = 1.0f;
    }
    lastTap_ = now;
    #endif

    glm::vec2 thrust = calcThrustVector();
    pRoidRage->pShip->updateThrust(thrust);
  } else {
    pRoidRage->pShip->stopThrust();
  }

}

void 
RoidRageGameSurvival::onMove(int index, int count, int* p, int* x, int* y, int* s) {
  //if (isShooting()) {
  //  return;
  //}

  //if (count == 1 && !isShooting()) {
    pRoidRage->pShip->updateThrust(calcThrustVector());
  //}
}

void 
RoidRageGameSurvival::onUp(int index, int count, int* p, int* x, int* y, int* s) {
  if (count == 1) {
    pRoidRage->pShip->stopThrust();
    multiplier_ = 1.0f;
  }

  //pRoidRage->leftShooting = false;
  //pRoidRage->rightShooting = false;
}

/*
bool 
RoidRageGameSurvival::isShooting() const {
  // side buttons should be better encapsulated
  const float barWidth = (pRoidRage->barWidth * 100.0f + 50.0f) * Display::getScale();

  // TODO transformation stack
  if (current_.x < -Display::getWidth()*0.5+barWidth) {
    pRoidRage->leftShooting = true;
    return true;
  }
  
  if (current_.x >  Display::getWidth()*0.5-barWidth) {
    pRoidRage->rightShooting = true;
    return true;
  }

  return false;
}
*/

glm::vec2 
RoidRageGameSurvival::calcThrustVector() const {
  float invert = pRoidRage->inverted?1.0f:-1.0f;
  return glm::normalize(current_) * (pRoidRage->thrustMag*350.0f + 50.0f) * invert * multiplier_;
}
  
//------------------------------------------------------------------------------

void 
RoidRageGameSurvival::onEvent(AndroidBack back) {
  getMachine()->transition<RoidRageMenu>(true);
}

//------------------------------------------------------------------------------
