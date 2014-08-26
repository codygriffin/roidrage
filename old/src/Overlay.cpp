//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Overlay.h"
#include "Ship.h"
#include "Display.h"
#include "LinearIndicator.h"
#include "RoidRage.h"
#include "OrthoCamera.h"
#include "Program.h"
#include "VertexBufferObject.h"
#include "Systems.h"
#include "Params.h"

#include <sstream>
#include <iomanip>

//------------------------------------------------------------------------------

using namespace roidrage;
using namespace pronghorn;

//------------------------------------------------------------------------------

Overlay::Overlay()
{
}

//------------------------------------------------------------------------------

void
Overlay::preRender(OrthoCamera* pCam)
{
}

//------------------------------------------------------------------------------

void  Overlay::onRender(OrthoCamera* pCam) {
  const float barWidth = (pRoidRage->barWidth * 100.0f + 50.0f) * Display::getScale();

  /*
  SolidQuad  left(0.0f, 0.0f, barWidth, Display::getHeight());
  if (pRoidRage->leftShooting) {
    left.getRenderPass().color = glm::vec4(0.50, 0.07f, 0.07, 0.3f);
  } else {
    left.getRenderPass().color = glm::vec4(0.07f, 0.07f, 0.07, 0.3f);
  }
  left.render(pCam);

  SolidQuad  right(Display::getWidth() - barWidth, 0.0f, barWidth, Display::getHeight());
  if (pRoidRage->rightShooting) {
    right.getRenderPass().color = glm::vec4(0.50, 0.07f, 0.07, 0.3f);
  } else {
    right.getRenderPass().color = glm::vec4(0.07f, 0.07f, 0.07, 0.3f);
  }
  right.render(pCam);
  */

  unsigned     score = std::min(pRoidRage->score, 9999999);
  if (pRoidRage->stage == 0) {

    auto s     = pRoidRage->gameTime/1000;
    auto m     = s  / 60;
         s     = s  % 60;
    auto h     = m  / 60;
         m     = m  % 60;

    std::stringstream timestamp;
    timestamp << std::setw(2) << std::setfill('0') << m << ":" 
              << std::setw(2) << std::setfill('0') << s;

    renderTextOverlay(0.0f, 0.0f, 16.0f * Display::getScale(), 
                     "survival\n"
                     "score: %\n"
                     "time: %\n"
                     "lives: %\n",
                     score, 
                     timestamp.str().c_str(), 
                     pRoidRage->lives);
  } else {
    renderTextOverlay(0.0f, 0.0f, 16.0f * Display::getScale(), 
                     "classic\n"
                     "stage: %\n"
                     "score: %\n"
                     "lives: %\n",
                     pRoidRage->stage, score, pRoidRage->lives);
  }


  float x = Display::getWidth() * 0.5f;
  float y = Display::getHeight() * 0.5f;

  LinearIndicator shield(x + 0.0f, y + 15.0f);
  shield.update(pRoidRage->pShip->Shield::mag); 
  shield.color([](float v) {
    if (v < 0.2f) {
      return glm::vec4(0.75f, 0.12f, 0.12, 1.0f);
    } else 
    if (v < 0.5f) {
      return glm::vec4(0.50, 0.50f, 0.25, 0.8f);
    } 
    else {
      return glm::vec4(0.42f, 0.42f, 0.85, 0.8f);
    }
  });
  shield.render(pCam);

};

//------------------------------------------------------------------------------
