//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "RoidRageGameOver.h"
#include "RoidRageMenu.h"

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
#include "Roid.h"
#include "Background.h"
#include "Overlay.h"

#include "Log.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace roidrage;

//------------------------------------------------------------------------------

RoidRageGameOver::RoidRageGameOver(RoidRage* pMachine, bool survival) 
  : RoidRage::State(pMachine) 
  , survival_ (survival)
  , secs_(0)
{ 
  pRoidRage->score = 0;

  pRoidRage->population.clear<PewPew*>();
  pRoidRage->population.clear<Text*>();

  Log::info("Population initialized for RoidRageGameOver");
}

//------------------------------------------------------------------------------

void 
RoidRageGameOver::onEvent(Tick tick) {
  pRoidRage->population.visit(&updateTime);
  pRoidRage->population.visit(&resetAcceleration);
  pRoidRage->population.visit(&updatePosition);
  pRoidRage->population.visit(&updateTransform); 

  // Orthographic projection, centered on our ship
  RenderState::pCam_->setPosition(pRoidRage->pShip->pos);
  pRoidRage->ortho = RenderState::pCam_->getOrthoMatrix(); 

  glClearColor(0.07f, 0.07f, 0.13f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Render our bits
  pRoidRage->population.visit(&renderOrtho); 

  // Render the overlay and text
  pRoidRage->ortho = RenderState::pCam_->getOverlayMatrix();

  // TODO centering text
 
  if (!survival_) {
    std::string gameOver("game over");

    renderTextOverlay(Display::getWidth()/2.0f - 8.0*gameOver.length(), 
                      Display::getHeight()/2.0f - 8.0f, 16.0f * Display::getScale(), 
                      gameOver.c_str());
  } else {
    auto s     = pRoidRage->gameTime/1000;
    auto m     = s  / 60;
         s     = s  % 60;
    auto h     = m  / 60;
         m     = m  % 60;
    std::stringstream gameOver;
    gameOver << "game over\n" 
             << std::setw(2) << std::setfill('0') << m << ":" 
             << std::setw(2) << std::setfill('0') << s;

    renderTextOverlay(Display::getWidth()/2.0f - 8.0*gameOver.str().length(), 
                      Display::getHeight()/2.0f - 8.0f, 16.0f * Display::getScale(), 
                      gameOver.str().c_str());
  }

  secs_ += pRoidRage->pShip->ms;
}

//------------------------------------------------------------------------------

void 
RoidRageGameOver::onEvent(Touch touch) {
  if (touch.action == Touch::down && secs_ > 500) {
    getMachine()->transition<RoidRageMenu>(true);
  }
}

//------------------------------------------------------------------------------

void 
RoidRageGameOver::onEvent(AndroidBack back) {
  getMachine()->transition<RoidRageMenu>(true);
}

//------------------------------------------------------------------------------
