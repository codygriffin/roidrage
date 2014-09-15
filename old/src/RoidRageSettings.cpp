//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "RoidRageSettings.h"
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
#include "AudioState.h"
#include "Display.h"

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

RoidRageSettings::RoidRageSettings(RoidRage* pMachine) 
  : RoidRage::State(pMachine)
  , settingsLayout_(0.0f, 0.0f, Display::getWidth(), Display::getHeight()) {

  //settingsLayout_.add(new Label("audio"));
  /*
  settingsLayout_.add(new Slider("sound volume",     
                                 [](float v) {
                                   pRoidRage->sfxVol = v;
                                   pRoidRage->pSfxPlayer->setVolume(pRoidRage->sfxVol);
                                 }, pRoidRage->sfxVol));
  settingsLayout_.add(new Slider("music volume",  
                                 [](float v) {
                                   pRoidRage->musVol = v;
                                   pRoidRage->pMusicPlayer->setVolume(pRoidRage->musVol);
                                 }, pRoidRage->musVol));

  settingsLayout_.add(new Checkbox("mute",   
                                  [](bool checked) {
                                    pRoidRage->mute = checked;
                                    if (pRoidRage->mute) {
                                      pRoidRage->pMusicPlayer->setVolume(0.0f);
                                      pRoidRage->pSfxPlayer->setVolume(0.0f);
                                    } else {
                                      pRoidRage->pMusicPlayer->setVolume(pRoidRage->musVol);
                                      pRoidRage->pSfxPlayer->setVolume(pRoidRage->sfxVol);
                                    }
                                  }, 
                                  pRoidRage->mute));
  */

  //settingsLayout_.add(new Label("gameplay"));
  settingsLayout_.add(new Slider("pew bar width", 
                                 [](float v) {
                                   pRoidRage->barWidth = v;
                                 }, pRoidRage->barWidth));
  settingsLayout_.add(new Slider("thrust power",  
                                 [](float v) {
                                   pRoidRage->thrustMag = v;
                                 }, pRoidRage->thrustMag));
  settingsLayout_.add(new Checkbox("thrust inversion",   
                                  [](bool checked) {
                                    pRoidRage->inverted = checked;
                                  }, 
                                  pRoidRage->inverted));

  static std::string graphicsQuality[] = {"low quality", "high quality"};
  //settingsLayout_.add(new Label("graphics"));
  settingsLayout_.add(new Selector(graphicsQuality[pRoidRage->sexy?1:0],
                                  [](Selector& s) {
                                    pRoidRage->sexy = (s.getValue() == 1);
                                    s.setLabel(graphicsQuality[s.getValue()]);  
                                  }, 
                                  2, pRoidRage->sexy?1:0));

  settingsLayout_.add(new Label(""));
  settingsLayout_.add(new Button("back",   
                                  [&](beta::GlfwMouseButton mouse) {
                                    getMachine()->transition<RoidRageMenu>(false);
                                  }));

  settingsLayout_.layout(0, glm::vec2(100.0f, 100.0f));

  pRoidRage->population.clear<Text*>();
  Log::info("Settings Display Initialized.");
}

//------------------------------------------------------------------------------

void
RoidRageSettings::onEvent(Tick tick) {
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

  settingsLayout_.render();
}

//------------------------------------------------------------------------------

void 
RoidRageSettings::onEvent(Touch touch) {
  settingsLayout_.touch(touch);

  if (touch.action == Touch::up) {
    pRoidRage->saveSettings();
  }
}

//------------------------------------------------------------------------------

void 
RoidRageSettings::onEvent(AndroidBack back) {
  getMachine()->transition<RoidRageMenu>(false);
}

//------------------------------------------------------------------------------
