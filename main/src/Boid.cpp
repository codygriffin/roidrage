//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Boid.h"
#include "RoidRage.h"

#include "Roid.h"
#include "PewPew.h"
#include "Bonus.h"
#include "Text.h"

#include "Log.h"
#include "AudioState.h"
#include "Population.h"
#include "Collision.h"
#include "OrthoCamera.h"


//------------------------------------------------------------------------------

using namespace roidrage;
using namespace pronghorn;


glm::vec2 Boid::avgPos[Boid::numSwarms];
glm::vec2 Boid::avgVel[Boid::numSwarms];
unsigned  Boid::count[Boid::numSwarms];

//------------------------------------------------------------------------------

Boid::Boid(float x, float y) 
: Quad                  (pRoidRage->pBoidTex.get(), pRoidRage->pProgram.get(), pRoidRage->pQuadVboPos.get())
, TimeDelta             ()
, TimeMs                (TimeMs::now())
, Position              ()
, Charge                (-1000.0f)
, Mass                  (360.0f)
, Radius                (80.0f)
, Thrust                (0.0f,0.0f)
, timeSinceLastPewMs_   (2000)
{
  Position::maxVel = 1.5f;
  glm::vec2 dir = glm::normalize(glm::vec2(rand()%100-50, rand()%100 - 50)); 
  Position::vel  =  dir * -(rand()%100/100.0f);
  Position::apos = atan2(dir.y, dir.x) * 360.0f/6.28f - 90.0f;
  Position::pos  = glm::vec2(x, y);
  swarm_ = rand() % numSwarms;
}

//------------------------------------------------------------------------------

Boid::~Boid() {
}

//------------------------------------------------------------------------------

void 
Boid::updateThrust(const glm::vec2& acc) {
  Thrust::vec    = acc;
  glm::vec2 dir = glm::normalize(acc);
  Position::apos = atan2(dir.y, dir.x) * 360.0f/6.28f - 90.0f;

  getRenderPass().pTex[0] = pRoidRage->pSpaceshipTex[0].get();
}

//------------------------------------------------------------------------------

void 
Boid::stopThrust() {
  Thrust::vec = glm::vec2(0.0f);
  getRenderPass().pTex[0] = pRoidRage->pSpaceshipTex[1].get();
}

//------------------------------------------------------------------------------

void 
Boid::shoot() {
  timeSinceLastPewMs_ += ms;
  if (timeSinceLastPewMs_ < 300) return; 

  //if (!pewpewLimiter_.consume()) {
  //  return;
  //}

  //pRoidRage->pSfxPlayer->pew(8);

  shoot(0.0f);
  timeSinceLastPewMs_  = 0;
}

//------------------------------------------------------------------------------
// Factor out 
void 
Boid::shoot(float angle) {

  // Create a new pewpew
  PewPew* pPewPew = new PewPew(Position::pos.x, Position::pos.y, PewPew::BOID);
  pPewPew->Position::vel = Position::vel + (glm::vec2(cos((Position::apos-90.0f-angle) * 6.28f/360.0f), 
                                                      sin((Position::apos-90.0f-angle) * 6.28f/360.0f)) * 2.0f);

  // Add this pewpew to the population
  pRoidRage->population.add(pPewPew);
}

//------------------------------------------------------------------------------
//
void 
Boid::onCollideWith(Boid*  pBoid,  Collision* pCollision) {
}

//------------------------------------------------------------------------------

void 
Boid::onCollideWith(PewPew* pPewPew, Collision* pCollision) {
  VALIDATE(pPewPew)
  VALIDATE(this)

  // No friendly fire
  if (pPewPew->getType() == PewPew::BOID) { 
    return;
  }

  invalidate();
  pPewPew->invalidate();
  
  const unsigned points = 350;
  pRoidRage->score += points;

  // Render Text
  Text* pText = new Text(pos.x, pos.y);
  pText->setText("%", points);
  pText->Position::vel = this->vel;
  pRoidRage->population.add(pText);
  
  //pRoidRage->pSfxPlayer->pew(150);
}
//------------------------------------------------------------------------------

void 
Boid::onCollideWith(Ship*  pShip,  Collision* pCollision) {

}

//------------------------------------------------------------------------------

int 
Boid::getSwarm() const {
  return swarm_;
}

//------------------------------------------------------------------------------

void
Boid::reset() {
  for (int i = 0; i < numSwarms; i++) {
    avgPos[i] = glm::vec2(0.0f, 0.0f);
    avgVel[i] = glm::vec2(0.0f, 0.0f);
    count[i]  = 0;
  }
}

//------------------------------------------------------------------------------

void
Boid::calcAverage() {
  for (int i = 0; i < Boid::numSwarms; i++) {
    if (Boid::count[i] > 0) {
      Boid::avgPos[i] /= Boid::count[i];
      Boid::avgVel[i] /= Boid::count[i];
    }
  }
}

//------------------------------------------------------------------------------
