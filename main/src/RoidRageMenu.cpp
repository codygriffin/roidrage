//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "RoidRageMenu.h"
#include "RoidRageScore.h"
#include "RoidRageSettings.h"
#include "RoidRageGameSurvival.h"
#include "RoidRageGameTesting.h"
#include "RoidRageGame.h"

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
#include "AudioState.h"

#include "Renderable.h"
#include "Ship.h"
#include "Boid.h"
#include "Roid.h"
#include "Background.h"

#include "Log.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace roidrage;

//------------------------------------------------------------------------------

RoidRageMenu::RoidRageMenu(RoidRage* pMachine, bool populateRoids) 
  : RoidRage::State(pMachine)
  , menuLayout_(Display::getWidth(), Display::getHeight()) {

  menuLayout_.add(new Button("classic",  []() {pRoidRage->transition<RoidRageGame>(false);}));
  menuLayout_.add(new Button("survival", []() {pRoidRage->transition<RoidRageGameSurvival>();}));
  menuLayout_.add(new Button("testing",  []() {pRoidRage->transition<RoidRageGameTesting>();}));
  menuLayout_.add(new Button("settings", []() {pRoidRage->transition<RoidRageSettings>();}));
  //menuLayout_.add(new Button("score",    []() {pRoidRage->transition<RoidRageScore>();}));
  menuLayout_.add(new Button("quit",     []() {pRoidRage->transition<ConfirmQuit>();}));

  menuLayout_.layout(0, glm::vec2(3.0f*Display::getWidth()/4.0f, Display::getHeight()/3.0f));

  pRoidRage->population.clear<Background*>();
  pRoidRage->population.clear<Ship*>();
  pRoidRage->population.clear<Boid*>();
  pRoidRage->population.clear<Bonus*>();
  pRoidRage->population.clear<PewPew*>();
  pRoidRage->population.clear<Text*>();
  pRoidRage->pShip = 0;
  srand(0); 

  if (populateRoids) {
    pRoidRage->population.clear<Roid*>();
    for (unsigned i = 0; i < 35; i++) {
      float roidX = (rand() % (2*(int)Position::max)) - (int)Position::max;
      float roidY = (rand() % (2*(int)Position::max)) - (int)Position::max;
      pRoidRage->population.add(new Roid(roidX, roidY));
    }
  }

  Log::info("Population initialized for RoidRageMenu");
  //pRoidRage->pMusicPlayer->loop(pRoidRage->menu.get(), 3353729);
}

//------------------------------------------------------------------------------

void
RoidRageMenu::onEvent(Tick tick) {
  pRoidRage->population.visit(&updateTime);
  pRoidRage->population.visit(&resetAcceleration);
  pRoidRage->population.visit(&enforceBoundaries);
  pRoidRage->population.visit(&updatePosition);
  pRoidRage->population.visit(&updateTransform); 

  // Orthographic projection, centered on our ship
  RenderState::pCam_->setPosition(glm::vec2(0.0f, 0.0f));
  pRoidRage->ortho = RenderState::pCam_->getOverlayMatrix();

  glClearColor(0.07f, 0.07f, 0.13f, 1.0f);
  glClearDepth(0.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  // Render our bits
  pRoidRage->population.visit(&renderOrtho); 
  
  menuLayout_.render();
}

//------------------------------------------------------------------------------

void 
RoidRageMenu::onEvent(Touch touch) {
  menuLayout_.touch(touch);
}

//------------------------------------------------------------------------------

void 
RoidRageMenu::onEvent(AndroidBack back) {
  pRoidRage->transition<ConfirmQuit>();
}

//------------------------------------------------------------------------------

ConfirmQuit::ConfirmQuit(RoidRage* pMachine) 
  : RoidRageMenu(pMachine, false) 
  , menuLayout_ (Display::getWidth(), Display::getHeight()) {

  menuLayout_.add(new Label("quitting?"));
  menuLayout_.add(new Button("yes", []() {pRoidRage->transition<Terminate>();}));
  menuLayout_.add(new Button("no",  []() {pRoidRage->transition<RoidRageMenu>(true);}));

  menuLayout_.layout(0, glm::vec2(1.0f*Display::getWidth()/4.0f, Display::getHeight()/3.0f));
}

//------------------------------------------------------------------------------

void 
ConfirmQuit::onEvent(Tick tick) {
  RoidRageMenu::onEvent(tick);

  SolidQuad bg(0.0f, 0.0f, Display::getWidth(), Display::getHeight());
  bg.getRenderPass().color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
  renderOverlay(&bg);

  menuLayout_.render();
}

//------------------------------------------------------------------------------

void 
ConfirmQuit::onEvent(Touch touch) {
  menuLayout_.touch(touch);
}

//------------------------------------------------------------------------------

void 
ConfirmQuit::onEvent(AndroidBack back) {
  pRoidRage->transition<RoidRageMenu>(false);
}

//------------------------------------------------------------------------------

ConfirmNewGame::ConfirmNewGame(RoidRage* pMachine) 
  : RoidRageMenu(pMachine, false) 
  , menuLayout_ (glm::vec2(0.5f*Display::getWidth(), 0.25f*Display::getHeight())) {

  menuLayout_.add(new Label("starting over?"));
  menuLayout_.add(new Button("yes", []() {pRoidRage->transition<RoidRageGame>(true);}));
  menuLayout_.add(new Button("no",  []() {pRoidRage->transition<RoidRageMenu>(true);}));

  menuLayout_.layout();
}

//------------------------------------------------------------------------------

void 
ConfirmNewGame::onEvent(Tick tick) {
  RoidRageMenu::onEvent(tick);

  // Dark, translucent background
  SolidQuad bg(0.0f, 0.0f, Display::getWidth(), Display::getHeight());
  bg.getRenderPass().color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
  renderOverlay(&bg);

  menuLayout_.render();
}

//------------------------------------------------------------------------------

void 
ConfirmNewGame::onEvent(Touch touch) {
  menuLayout_.touch(touch);
}

//------------------------------------------------------------------------------

void 
ConfirmNewGame::onEvent(AndroidBack back) {
  pRoidRage->transition<RoidRageMenu>(false);
}

//------------------------------------------------------------------------------
