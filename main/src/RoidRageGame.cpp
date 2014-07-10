//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "RoidRageGame.h"
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

#include "AudioState.h"
#include "Log.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

#define THRUST_MULTIPLIER

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace roidrage;

//------------------------------------------------------------------------------

RoidRageGame::RoidRageGame(RoidRage* pMachine, bool newGame) 
  : RoidRage::State(pMachine) 
  , inverted_   (true) 
  , multiplier_ (1.0f)
  , lastTap_    (0.0f)
  , gen_        (1)
{ 
  pRoidRage->tutorial.push("classic");
  pRoidRage->tutorial.push("clear the level");
  pRoidRage->tutorial.push("good luck!");

  pRoidRage->lives    = 3; 
  pRoidRage->stage    = 1; 
  pRoidRage->gameTime = 0; 
  pRoidRage->score    = 0; 

  pRoidRage->population.clear();

  timerReactor_.setPeriodic(std::chrono::seconds(15), [&] () {
    gameQueue_.enqueue( [&] () {
      spawnBoids();
    });
  });

  timerReactor_.setPeriodic(std::chrono::seconds(5), [&] () {
    gameQueue_.enqueue( [&] () {
      spawnTutorialText();
    });
  });

  populate();
  announce();

  //pRoidRage->pMusicPlayer->loop(pRoidRage->music.get(), 3351259);
}

//------------------------------------------------------------------------------

void
RoidRageGame::announce() { 
  // announce stage
  Text* pStageText = new Text(pRoidRage->pShip->pos.x + 100.0f, 
                              pRoidRage->pShip->pos.y + 100.0f);
  pStageText->setText("stage %", pRoidRage->stage);
  pStageText->expiresInMs = 3000;
  pRoidRage->population.add(pStageText);
}

//------------------------------------------------------------------------------

void
RoidRageGame::populate() { 
  Log::info("Populating classic game...");

  // TODO singletons in population?
  if (!pRoidRage->pShip) {
    pRoidRage->pShip = new Ship();
    pRoidRage->population.add(new Background());
    pRoidRage->population.add(pRoidRage->pShip);
  }

  srand(pRoidRage->stage); 
  for (unsigned i = 0; i < pRoidRage->stage; i++) {
    float roidX = (rand() % (2*(int)Position::max)) - (int)Position::max;
    float roidY = (rand() % (2*(int)Position::max)) - (int)Position::max;
    while (glm::length(glm::vec2(roidX, roidY) - pRoidRage->pShip->pos) < 800) {
      roidX = (rand() % (2*(int)Position::max)) - (int)Position::max;
      roidY = (rand() % (2*(int)Position::max)) - (int)Position::max;
    }

    pRoidRage->population.add(new Roid(roidX, roidY));
  }

  Log::info("Population initialized...");
}


//------------------------------------------------------------------------------

#define DUMP_VEC2(n,v) Log::debug(#n "(%, %)", v.x, v.y);

// XXX Ugly MACRO for cleaning up deleted members of the population
#define CLEANUP(x) for (auto it : pRoidRage->population.getRelation<x*>().relation_) { \
    x* p##x = std::get<0>(it); \
    if (!p##x->isValid()) { \
      delete p##x; \
      pRoidRage->population.remove(p##x); \
    } \
  } \

// XXX Ugly MACRO for removing members of the population
#define REMOVE(x) for (auto it : pRoidRage->population.getRelation<x*>().relation_) { \
    x* p##x = std::get<0>(it); \
    if (!p##x->isValid()) { \
      pRoidRage->population.remove(p##x); \
    } \
  } \

void 
RoidRageGame::onEvent(Tick tick) {
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

  while (gameQueue_.work());

  // TODO exit conditions should be better specified

  // If we have no more lives, game over
  if (pRoidRage->population.getRelation<Ship*>().count() == 0) {
    pRoidRage->transition<RoidRageGameOver>();
  }

  // We've beat this stage - move on
  if (pRoidRage->population.getRelation<Roid*>().count()  == 0) {
    pRoidRage->stage++;
    populate();
    announce();
  }
}

//------------------------------------------------------------------------------

void 
RoidRageGame::spawnTutorialText() {
  if (pRoidRage->tutorial.empty()) {
    return;
  }

  Text* pText = new Text(pRoidRage->pShip->pos.x-400.0f, pRoidRage->pShip->pos.y-100.0f);
  pText->expiresInMs = 5000;
  pText->setText("%", pRoidRage->tutorial.front());
  pRoidRage->population.add(pText);
  pRoidRage->tutorial.pop();
}

//------------------------------------------------------------------------------

void 
RoidRageGame::spawnBoids() {
  Log::info("Spawning some boids!");
  glm::vec4 pos(6000.0f, 6000.0f, 0.0f, 0.0f);
  pos = glm::rotate(glm::mat4(), (float)(rand() % 3600)/10.0f, 
    glm::vec3(0.0f, 0.0f, 1.0f)) * pos;

  unsigned numBoids = ceil(sqrt(pRoidRage->stage));

  // lets get a warning
  if (numBoids > 0) {
    Text* pText = new Text(pRoidRage->pShip->pos.x+400.0f, pRoidRage->pShip->pos.y+100.0f);
    pText->expiresInMs = 3000;
    pText->setText("INCOMING");
    pText->getRenderPass().color = glm::vec4(1.0f, 0.8f, 0.3f, 1.0f);
    pRoidRage->population.add(pText);
  }

  // Add our boids
  while (numBoids > 0) {
    Boid* pBoid = new Boid(pos.x + (numBoids*50.0f), pos.y + (numBoids*50.0f));  
    pRoidRage->population.add(pBoid);
    numBoids--;
  }
}

//------------------------------------------------------------------------------

void 
RoidRageGame::onEvent(Touch touch) {
  const float x = touch.xs[touch.index];
  const float y = touch.ys[touch.index];

  current_ = glm::vec2(x - (Display::getWidth()  * 0.5f), 
                       y - (Display::getHeight() * 0.5f));

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
RoidRageGame::onDown(int index, int count, int* p, int* x, int* y, int* s) {
  if (isShooting()) {
    pRoidRage->pShip->shoot();
    return;
  }

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
RoidRageGame::onMove(int index, int count, int* p, int* x, int* y, int* s) {
  if (isShooting()) {
    return;
  }

  if (count == 1 && !isShooting()) {
    pRoidRage->pShip->updateThrust(calcThrustVector());
  }
}

void 
RoidRageGame::onUp(int index, int count, int* p, int* x, int* y, int* s) {
  if (count == 1) {
    pRoidRage->pShip->stopThrust();
    multiplier_ = 1.0f;
  }

  pRoidRage->leftShooting = false;
  pRoidRage->rightShooting = false;
}

bool 
RoidRageGame::isShooting() const {
  // side buttons should be better encapsulated
  const float barWidth = (pRoidRage->barWidth * 100.0f + 50.0f) * Display::getScale();

  // TODO transformation stack
  if (current_.x < -Display::getWidth()*0.5f+barWidth) {
    pRoidRage->leftShooting = true;
    return true;
  }
  
  if (current_.x >  Display::getWidth()*0.5f-barWidth) {
    pRoidRage->rightShooting = true;
    return true;
  }

  return false;
}

glm::vec2 
RoidRageGame::calcThrustVector() const {
  float invert = pRoidRage->inverted?1.0f:-1.0f;
  return glm::normalize(current_) * (pRoidRage->thrustMag*350.0f + 50.0f) * invert * multiplier_;
}
  
//------------------------------------------------------------------------------

void 
RoidRageGame::onEvent(AndroidBack back) {
  getMachine()->transition<RoidRageMenu>(true);
}

//------------------------------------------------------------------------------
