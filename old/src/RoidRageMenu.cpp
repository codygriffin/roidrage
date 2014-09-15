//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "RoidRageMenu.h"
#include "RoidRageScore.h"
#include "RoidRageSettings.h"
#include "RoidRageGameSurvival.h"
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

#include "Panel.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace roidrage;

//------------------------------------------------------------------------------

RoidRageMenu::RoidRageMenu(RoidRage* pMachine, bool populateRoids) 
  : RoidRage::State(pMachine)
  , menuLayout_(0.0f, 0.0f, Display::getWidth(), Display::getHeight()) {

  auto panel = new ui::Panel(glm::vec4(100.0f, 100.0f, 400.0f, 400.0f),
                             glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
  panel->add(new Label("new menu", 24.0f));
  panel->add(new Button("classic",  [](beta::GlfwMouseButton mouse) {pRoidRage->transition<RoidRageGame>(false);}, 24.0f));
  panel->add(new Button("survival", [](beta::GlfwMouseButton mouse) {pRoidRage->transition<RoidRageGameSurvival>();}, 24.0f));
  panel->add(new Button("settings", [](beta::GlfwMouseButton mouse) {pRoidRage->transition<RoidRageSettings>();}, 24.0f));
  //menuLayout_.add(new Button("score",    []() {pRoidRage->transition<RoidRageScore>();}));
  panel->add(new Button("quit",     [](beta::GlfwMouseButton mouse) {pRoidRage->transition<ConfirmQuit>();}, 24.0f));

  menuLayout_.add(panel);
  menuLayout_.layout();

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
  
  auto projection = glm::ortho(0.0f, 
                               roidrage::Display::getWidth(), 
                              -roidrage::Display::getHeight(), 
                               0.0f, 
                              -1.0f,  1.0f);
  menuLayout_.render(projection, glm::vec2());
}

//------------------------------------------------------------------------------

void 
RoidRageMenu::onEvent(Touch touch) {
  beta::GlfwMouseButton mouse;
  mouse.x = touch.xs[0];
  mouse.y = touch.ys[0];
  Log::debug("touch: %, %", mouse.x, mouse.y);
  menuLayout_.event(mouse);
}

//------------------------------------------------------------------------------

void 
RoidRageMenu::onEvent(AndroidBack back) {
  pRoidRage->transition<ConfirmQuit>();
}

//------------------------------------------------------------------------------

ConfirmQuit::ConfirmQuit(RoidRage* pMachine) 
  : RoidRageMenu(pMachine, false) 
  , menuLayout_ (300.0f, 300.0f, Display::getWidth(), Display::getHeight()) {

  menuLayout_.add(new Label("quitting?", 24.0f));
  menuLayout_.add(new Button("yes", [](beta::GlfwMouseButton mouse) {pRoidRage->transition<Terminate>();}, 24.0f));
  menuLayout_.add(new Button("no",  [](beta::GlfwMouseButton mouse) {pRoidRage->transition<RoidRageMenu>(true);}, 24.0f));

  menuLayout_.layout();
}

//------------------------------------------------------------------------------

void 
ConfirmQuit::onEvent(Tick tick) {
  RoidRageMenu::onEvent(tick);

  SolidQuad bg(0.0f, 0.0f, Display::getWidth(), Display::getHeight());
  bg.getRenderPass().color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
  renderOverlay(&bg);

  auto projection = glm::ortho(0.0f, 
                               roidrage::Display::getWidth(), 
                              -roidrage::Display::getHeight(), 
                               0.0f, 
                              -1.0f,  1.0f);
  menuLayout_.render(projection, glm::vec2());
}

//------------------------------------------------------------------------------

void 
ConfirmQuit::onEvent(Touch touch) {
  //menuLayout_.touch(touch);
  beta::GlfwMouseButton mouse;
  mouse.x = touch.xs[0];
  mouse.y = touch.ys[0];
  Log::debug("touch: %, %", mouse.x, mouse.y);
  menuLayout_.event(mouse);
}

//------------------------------------------------------------------------------

void 
ConfirmQuit::onEvent(AndroidBack back) {
  pRoidRage->transition<RoidRageMenu>(false);
}

//------------------------------------------------------------------------------

ConfirmNewGame::ConfirmNewGame(RoidRage* pMachine) 
  : RoidRageMenu(pMachine, false) 
  , menuLayout_ (0.0f, 0.0f, 0.5f*Display::getWidth(), 0.25f*Display::getHeight()) {

  menuLayout_.add(new Label("starting over?"));
  menuLayout_.add(new Button("yes", [](beta::GlfwMouseButton mouse) {pRoidRage->transition<RoidRageGame>(true);}));
  menuLayout_.add(new Button("no",  [](beta::GlfwMouseButton mouse) {pRoidRage->transition<RoidRageMenu>(true);}));

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

  auto projection = glm::ortho(0.0f, 
                               roidrage::Display::getWidth(), 
                              -roidrage::Display::getHeight(), 
                               0.0f, 
                              -1.0f,  1.0f);
  menuLayout_.render(projection, glm::vec2());
}

//------------------------------------------------------------------------------

void 
ConfirmNewGame::onEvent(Touch touch) {
  //menuLayout_.touch(touch);
  beta::GlfwMouseButton mouse;
  mouse.x = touch.xs[0];
  mouse.y = touch.ys[0];
  Log::debug("touch: %, %", mouse.x, mouse.y);
  menuLayout_.event(mouse);
}

//------------------------------------------------------------------------------

void 
ConfirmNewGame::onEvent(AndroidBack back) {
  pRoidRage->transition<RoidRageMenu>(false);
}

//------------------------------------------------------------------------------
