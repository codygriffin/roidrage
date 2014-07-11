//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_ROIDRAGE_H
#define INCLUDED_ROIDRAGE_H

#ifndef INCLUDED_STATEMACHINE_H
#include "StateMachine.h"
#endif

#ifndef INCLUDED_POPULATION_H
#include "Population.h"
#endif 

#ifndef INCLUDED_RENDERABLE_H
#include "Renderable.h"
#endif 

#ifndef INCLUDED_LAYOUT_H
#include "Layout.h"
#endif 

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif 

#ifndef INCLUDED_QUEUE
#include <queue>
#define INCLUDED_QUEUE
#endif 

#include "Asset.h"

//#define FX_GLOW

namespace corvid {
  struct WorkQueue;
  struct TimerReactor;
}

namespace pronghorn {
  struct VertexShader;
  struct FragmentShader;
  struct Program;
  struct Framebuffer;
  struct Texture;
  struct VertexBufferObject;
}

//------------------------------------------------------------------------------

namespace roidrage {

//------------------------------------------------------------------------------

struct Text;
struct PewPew;
struct Bonus;
struct Ship;
struct Boid;
struct Roid;
struct Roidar;
struct Background;
struct OrthoCamera;
struct Frame;

//struct AudioEngine;
//struct AudioPlayer;

//------------------------------------------------------------------------------

typedef pronghorn::Population<
  roidrage::Ship*,
  roidrage::Boid*,
  roidrage::Text*,
  roidrage::Roid*,
  roidrage::PewPew*, 
  roidrage::Bonus*,
  roidrage::Background*
> RoidRagePopulation;

//------------------------------------------------------------------------------

struct Tick {};
struct Render {};

struct AndroidBack {};

struct GlfwKey         { int key; int scancode; int action; int mods; };
struct GlfwMouseButton { int button; int action; int mods; };
struct GlfwMouseMove   { double x; double y; };

//------------------------------------------------------------------------------

typedef corvid::Machine<
  Tick, 
  Render, 
  Touch, 
  AndroidBack,
  GlfwKey,
  GlfwMouseButton,
  GlfwMouseMove
> RoidRage;

struct RoidRageMachine : public RoidRage
{
  RoidRageMachine(int width, int height, float s = 1.0f);

  void saveProgress();
  void loadProgress();

  void saveSettings();
  void loadSettings();

  // Region
  int   width;
  int   height;
  float dpiScaling;

  // Game Population
  RoidRagePopulation population;

  // Resources TODO - move this shit out
  std::unique_ptr<pronghorn::VertexShader>       pVertShader;
  std::unique_ptr<pronghorn::VertexShader>       pCharVertShader;
  std::unique_ptr<pronghorn::VertexShader>       pOverlayVertShader;

  std::unique_ptr<pronghorn::FragmentShader>     pBgFragShader;
  std::unique_ptr<pronghorn::FragmentShader>     pFragShader;
  std::unique_ptr<pronghorn::FragmentShader>     pSolidShader;
  std::unique_ptr<pronghorn::FragmentShader>     pShieldShader;
  std::unique_ptr<pronghorn::FragmentShader>     pCharShader;
  std::unique_ptr<pronghorn::FragmentShader>     pOverlayFragShader;
  std::unique_ptr<pronghorn::FragmentShader>     pGlowFragShader;

  std::unique_ptr<pronghorn::Program>            pProgram;
  std::unique_ptr<pronghorn::Program>            pSolidProgram;
  std::unique_ptr<pronghorn::Program>            pShieldProgram;
  std::unique_ptr<pronghorn::Program>            pBgProgram;
  std::unique_ptr<pronghorn::Program>            pCharProgram;
  std::unique_ptr<pronghorn::Program>            pOverlayProgram;
  std::unique_ptr<pronghorn::Program>            pGlowProgram;

  std::unique_ptr<pronghorn::VertexBufferObject> pQuadVboPos;
  std::unique_ptr<pronghorn::VertexBufferObject> pQuadVboTex;
  std::unique_ptr<pronghorn::VertexBufferObject> pOverlayVbo;

  std::unique_ptr<pronghorn::Texture>            pPewPewTex[3];
  std::unique_ptr<pronghorn::Texture>            pBonusTex[4];
  std::unique_ptr<pronghorn::Texture>            pSpaceshipTex[2];
  std::unique_ptr<pronghorn::Texture>            pRoidTex[3];
  std::unique_ptr<pronghorn::Texture>            pBoidTex;
  std::unique_ptr<pronghorn::Texture>            pFont;

  std::queue<const char*>                        tutorial;

  glm::mat4             ortho; 
  roidrage::Ship*       pShip;
  static const int      radarChunks = 12;
  float                 radar[radarChunks];
  float                 trajectory[50];
  int                   trajectoryPoints;

  bool                  newPlayer;

  // Some global game state
  int stage;
  int score;
  int lives; 
  int gameTime; 
  int overallScore;
  int scores[1000];

  // Settings
  float sfxVol;
  float musVol;
  bool  mute;
  float barWidth;
  float thrustMag;
  bool  inverted;
  bool  sexy;

  std::unique_ptr<char[]>      music; 
  std::unique_ptr<char[]>      menu; 
  //std::unique_ptr<AudioEngine> pAudioEngine;
  //std::unique_ptr<AudioPlayer> pSfxPlayer;
  //std::unique_ptr<AudioPlayer> pMusicPlayer;

  bool rightShooting;
  bool leftShooting;
  
  static std::string localDir;
private:
};

//------------------------------------------------------------------------------

} // namespace

extern roidrage::RoidRageMachine* pRoidRage;

//------------------------------------------------------------------------------

#endif
