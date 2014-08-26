//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "RoidRageScore.h"
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
#include "Display.h"
#include "RenderState.h"

#include "Renderable.h"
#include "Ship.h"
#include "Roid.h"
#include "Background.h"
#include "Slider.h"
#include "Selector.h"
#include "Checkbox.h"

#include "Log.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace roidrage;

//------------------------------------------------------------------------------

RoidRageScore::RoidRageScore(RoidRage* pMachine) 
  : RoidRage::State(pMachine)
  , scoreLayout_(Display::getWidth(), Display::getHeight()) 
  , first_ (1) 
  , page_  (0)  
  , max_   (100) {

  static std::string gameType[] = {"classic", "survival"};
  //settingsLayout_.add(new Label("graphics"));
  scoreLayout_.add(new Selector(gameType[0],
                                  [](Selector& s) {
                                    s.setLabel(gameType[s.getValue()]);  
                                  }, 
                                  2, 0));

  for (int i = 1; i <= 10; i++) {
    std::stringstream str;
    str << std::setw(4) << i << std::setw(10) << rand() % 999999;
    scoreLayout_.add(new Label(str.str()));
  }
  
  //scoreLayout_.layout(0, glm::vec2(0.0f, 96.0f));
  scoreLayout_.layout();

  pRoidRage->population.clear<Text*>();
  Log::info("Score Display Initialized.");
}

//------------------------------------------------------------------------------

void
RoidRageScore::onEvent(Tick tick) {
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

  scoreLayout_.render();
}

//------------------------------------------------------------------------------

void 
RoidRageScore::onEvent(Touch touch) {
  scoreLayout_.touch(touch);

  if (touch.action == Touch::down) {
  }
}

//------------------------------------------------------------------------------

void 
RoidRageScore::onEvent(AndroidBack back) {
  getMachine()->transition<RoidRageMenu>(false);
}

//------------------------------------------------------------------------------
