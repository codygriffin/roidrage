//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "RoidRage.h"

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
#include "Boid.h"
#include "Roid.h"
#include "Background.h"

#include "Log.h"
#include "AssetManager.h"

#include "RoidRageGame.h"
#include "RoidRageMenu.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace roidrage;

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------


RoidRageMachine* pRoidRage = 0;
std::string RoidRageMachine::localDir = "";

RoidRageMachine::RoidRageMachine(int w, int h, float s) 
  : width  (w) 
  , height (h) 
  , dpiScaling (s) 
  , pShip  (0)
  , trajectoryPoints (0)
  , lives  (1) 
  , stage  (1) 
  , gameTime (0) 
  , score  (0) 
  , overallScore (0) 
  , sfxVol (0.4f)
  , musVol (0.5f)
  , mute   (false)
  , barWidth(0.5f)
  , thrustMag(0.5f)
  , sexy     (true)
  , inverted (true)
  , newPlayer(true)
  , rightShooting (false)
  , leftShooting  (false)
{ 

  // Initialize scoreboard
  for (int i = 0; i < 1000; i++) {
    scores[i] = 0;
  }

  // TODO return {width height data} tuple via JNI
  unsigned w_, h_;
  auto pBonus01Data     = AssetManager::loadBitmap("assets/png/nugget01.png",  w_, h_);
  auto pBonus02Data     = AssetManager::loadBitmap("assets/png/charge.png",    w_, h_);
  auto pBonus03Data     = AssetManager::loadBitmap("assets/png/triple.png",    w_, h_);
  auto pBonus04Data     = AssetManager::loadBitmap("assets/png/extra.png",     w_, h_);
  auto pPewPew01Data    = AssetManager::loadBitmap("assets/png/pewpew01.png",  w_, h_);
  auto pPewPew02Data    = AssetManager::loadBitmap("assets/png/pewpew02.png",  w_, h_);
  auto pPewPew03Data    = AssetManager::loadBitmap("assets/png/pewpew03.png",  w_, h_);
  auto pSpaceship01Data = AssetManager::loadBitmap("assets/png/ship01.png",    w_, h_);
  auto pSpaceship02Data = AssetManager::loadBitmap("assets/png/ship02.png",    w_, h_);
  auto pRoid01Data      = AssetManager::loadBitmap("assets/png/astroid01.png", w_, h_);
  auto pRoid02Data      = AssetManager::loadBitmap("assets/png/astroid02.png", w_, h_);
  auto pRoid03Data      = AssetManager::loadBitmap("assets/png/astroid03.png", w_, h_);
  auto pBoidData        = AssetManager::loadBitmap("assets/png/boid01.png",    w_, h_);
  auto pFontData        = AssetManager::loadBitmap("assets/png/menlo24.png",   w_, h_);
  
  pVertShader.reset       (new VertexShader  ("assets/gpu/transform.vp"));
  pFragShader.reset       (new FragmentShader("assets/gpu/texture.fp"));
  pSolidShader.reset      (new FragmentShader("assets/gpu/solid.fp"));
  pShieldShader.reset     (new FragmentShader("assets/gpu/shield.fp"));
  pBgFragShader.reset     (new FragmentShader("assets/gpu/background.fp"));
  pCharVertShader.reset   (new VertexShader  ("assets/gpu/char.vp"));
  pCharShader.reset       (new FragmentShader("assets/gpu/char.fp"));
  pOverlayVertShader.reset(new VertexShader  ("assets/gpu/overlay.vp"));
  pOverlayFragShader.reset(new FragmentShader("assets/gpu/overlay.fp"));
  pGlowFragShader.reset   (new FragmentShader("assets/gpu/glow.fp"));

  Log::info("Assets Loaded");

  // Allocate and initialize resource buffers
  pBonusTex[0].reset      (new Texture(256, 256, Texture::RGBA,  pBonus01Data.get()));
  pBonusTex[1].reset      (new Texture(256, 256, Texture::RGBA,  pBonus02Data.get()));
  pBonusTex[2].reset      (new Texture(256, 256, Texture::RGBA,  pBonus03Data.get()));
  pBonusTex[3].reset      (new Texture(256, 256, Texture::RGBA,  pBonus04Data.get()));
  pPewPewTex[0].reset     (new Texture(256, 256, Texture::RGBA,  pPewPew01Data.get()));
  pPewPewTex[1].reset     (new Texture(256, 256, Texture::RGBA,  pPewPew02Data.get()));
  pPewPewTex[2].reset     (new Texture(256, 256, Texture::RGBA,  pPewPew03Data.get()));
  pSpaceshipTex[0].reset  (new Texture(512, 512, Texture::RGBA,  pSpaceship01Data.get()));
  pSpaceshipTex[1].reset  (new Texture(512, 512, Texture::RGBA,  pSpaceship02Data.get()));
  pRoidTex[0].reset       (new Texture(512, 512, Texture::RGBA,  pRoid01Data.get()));
  pRoidTex[1].reset       (new Texture(512, 512, Texture::RGBA,  pRoid02Data.get()));
  pRoidTex[2].reset       (new Texture(512, 512, Texture::RGBA,  pRoid03Data.get()));
  pBoidTex.reset          (new Texture(512, 512, Texture::RGBA,  pBoidData.get()));
  pFont.reset             (new Texture(512, 512, Texture::RGBA,  pFontData.get()));

  pProgram.reset          (new Program(*pVertShader,        *pFragShader));
  pSolidProgram.reset     (new Program(*pVertShader,        *pSolidShader));
  pShieldProgram.reset    (new Program(*pVertShader,        *pShieldShader));
  pBgProgram.reset        (new Program(*pVertShader,        *pBgFragShader));
  pCharProgram.reset      (new Program(*pCharVertShader,    *pCharShader));
  pOverlayProgram.reset   (new Program(*pOverlayVertShader, *pOverlayFragShader, Program::lineLoop));
  pGlowProgram.reset      (new Program(*pVertShader,        *pGlowFragShader));


  const GLfloat pOverlayTri[] = {0.000f,  0.000f, 
                                 0.866f, -0.500f,
                                 1.000f,  0.000f};
  pOverlayVbo.reset       (new VertexBufferObject(6*sizeof(GLfloat), pOverlayTri));

  const GLfloat pQuad[] = {1.0,  1.0, 1.0, 1.0,
                           1.0, -1.0, 1.0, 0.0,
                          -1.0, -1.0, 0.0, 0.0,
                          -1.0,  1.0, 0.0, 1.0};
  pQuadVboPos.reset       (new VertexBufferObject(16*sizeof(GLfloat), pQuad));

  Display::reset(w, h, s);
  Log::info("Display initialized");

  RenderState::reset(width, height);
  Log::info("RenderState initialized");

  //pAudioEngine.reset(new AudioEngine());
  //Log::info("AudioEngine initialized");

  //pSfxPlayer.reset(new AudioPlayer(*pAudioEngine));
  //pSfxPlayer->minVol = -8000;
  //pSfxPlayer->maxVol = 100;
  //pMusicPlayer.reset(new AudioPlayer(*pAudioEngine, 2, 1, AudioPlayer::rate48k));
  //pMusicPlayer->bufferCallback_ = [&]() //{
 //   pMusicPlayer->music();
 // };
  //Log::info("AudioPlayers initialized");

  loadSettings();

  music = AssetManager::loadText("sfx/music.raw");
  menu  = AssetManager::loadText("sfx/menu.raw");

  if (mute) {
    //pMusicPlayer->setVolume(0.0f);
    //pSfxPlayer->setVolume(0.0f);
  } else {
    //pMusicPlayer->setVolume(musVol);
    //pSfxPlayer->setVolume(sfxVol);
  }
}

//------------------------------------------------------------------------------

void
RoidRageMachine::saveProgress() {
  std::string path = localDir + "/progress.2";
  Log::info("Saving progress to %", path.c_str());
  std::ofstream progress(path, std::ios::binary);

  Log::info("Saving stage: %", stage);
  progress.write(reinterpret_cast<char*>(&stage), sizeof(int));

  Log::info("Saving lives: %", stage);
  progress.write(reinterpret_cast<char*>(&lives), sizeof(int));

  Log::info("Saving score: %", score);
  progress.write(reinterpret_cast<char*>(&score), sizeof(int));

  Log::info("Saving scores...");
  progress.write(reinterpret_cast<char*>(scores), sizeof(int) * stage);
}

//------------------------------------------------------------------------------

void
RoidRageMachine::loadProgress() {
  std::string path = localDir + "/progress.2";
  Log::info("Loading progress from %", path.c_str());
  std::ifstream progress(path, std::ios::binary);

  if (!progress.is_open()) {
    Log::info("New game...");
    return;
  }

  progress.read(reinterpret_cast<char*>(&stage), sizeof(int));
  Log::info("Loading stage: %", stage);

  progress.read(reinterpret_cast<char*>(&lives), sizeof(int));
  Log::info("Loading lives: %", lives);

  progress.read(reinterpret_cast<char*>(&score), sizeof(int));
  Log::info("Loading score: %", score);

  progress.read(reinterpret_cast<char*>(scores), sizeof(int) * stage);
  Log::info("Loading scores...");
  
  newPlayer = false;
}

//------------------------------------------------------------------------------

void
RoidRageMachine::saveSettings() {
  std::string path = localDir + "/settings.3";
  Log::info("Saving settings to %", path.c_str());
  std::ofstream settings(path, std::ios::binary);

  Log::info("Saving sfxVol: %", sfxVol);
  settings.write(reinterpret_cast<char*>(&sfxVol), sizeof(float));

  Log::info("Saving musVol: %", musVol);
  settings.write(reinterpret_cast<char*>(&musVol), sizeof(float));

  Log::info("Saving mute: %", inverted);
  settings.write(reinterpret_cast<char*>(&mute),      sizeof(bool));

  Log::info("Saving barWidth: %", barWidth);
  settings.write(reinterpret_cast<char*>(&barWidth), sizeof(float));

  Log::info("Saving thrustMag: %", thrustMag);
  settings.write(reinterpret_cast<char*>(&thrustMag), sizeof(float));

  Log::info("Saving thrustinversion: %", inverted);
  settings.write(reinterpret_cast<char*>(&inverted),      sizeof(bool));

  Log::info("Saving sexy: %", sexy);
  settings.write(reinterpret_cast<char*>(&sexy),      sizeof(bool));
}

//------------------------------------------------------------------------------

void
RoidRageMachine::loadSettings() {
  std::string path = localDir + "/settings.3";
  Log::info("Loading settings from %", path.c_str());
  std::ifstream settings(path, std::ios::binary);
  if (!settings.is_open()) {
    return;
  }

  settings.read(reinterpret_cast<char*>(&sfxVol), sizeof(float));
  Log::info("Loading sfxVol: %", sfxVol);

  settings.read(reinterpret_cast<char*>(&musVol), sizeof(float));
  Log::info("Loading musVol: %", musVol);

  settings.read(reinterpret_cast<char*>(&mute), sizeof(bool));
  Log::info("Loading mute...");

  settings.read(reinterpret_cast<char*>(&barWidth), sizeof(float));
  Log::info("Loading barWidth: %", barWidth);

  settings.read(reinterpret_cast<char*>(&thrustMag), sizeof(float));
  Log::info("Loading thrustMag...");

  settings.read(reinterpret_cast<char*>(&inverted), sizeof(bool));
  Log::info("Loading inversion...");

  settings.read(reinterpret_cast<char*>(&sexy), sizeof(bool));
  Log::info("Loading sexy...");
}

//------------------------------------------------------------------------------
