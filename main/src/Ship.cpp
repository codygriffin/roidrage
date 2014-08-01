//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Ship.h"
#include "RoidRage.h"

#include "Roid.h"
#include "Boid.h"
#include "PewPew.h"
#include "Bonus.h"
#include "Text.h"

#include "LinearIndicator.h"

#include "Log.h"
#include "AudioState.h"
#include "Population.h"
#include "Collision.h"
#include "OrthoCamera.h"

// Ship dies on roid hit
#define SUDDEN_DEATH

//------------------------------------------------------------------------------

using namespace roidrage;
using namespace pronghorn;

//------------------------------------------------------------------------------

Ship::Ship() 
: Quad                  (pRoidRage->pSpaceshipTex[1].get(), pRoidRage->pProgram.get(), pRoidRage->pQuadVboPos.get())
, TimeDelta             ()
, TimeMs                (TimeMs::now())
, Position              ()
, Charge                (-1000.0f)
, Mass                  (360.0f)
, Radius                (80.0f)
, Thrust                (0.0f,0.0f)
, Attitude              (0.0f)
, Shield                (1.0f)
, Fuel                  (1.0f)
, points_               (0)
, hasCharge_            (false)
, hasTriple_            (false)
, hasShield_            (false)
, chargeAgeMs_          (0)
, chargeExpirationMs_   (20000)
, tripleAgeMs_          (0)
, tripleExpirationMs_   (20000)
, shieldAgeMs_          (10000)
, pewpewLimiter_        (0.25f, 15.0f, 8.0f)
{
  Position::maxVel = 2.0f;
  glm::vec2 dir = glm::normalize(glm::vec2(rand()%100-50, rand()%100 - 50)); 
  Position::vel  =  dir * -(rand()%100/400.0f);
  Position::apos = atan2(dir.y, dir.x) * 360.0f/6.28f - 90.0f;
}

//------------------------------------------------------------------------------

Ship::~Ship() {
}

//------------------------------------------------------------------------------

void 
Ship::updateThrust(float thrust) {
  Thrust::vec = glm::vec2(cos((Position::apos-90.0f) * 6.28f/360.0f), 
                          sin((Position::apos-90.0f) * 6.28f/360.0f)) * thrust;
  getRenderPass().pTex[0] = pRoidRage->pSpaceshipTex[0].get();
}

//------------------------------------------------------------------------------

void 
Ship::updateAttitude(float dir) {
  Attitude::dir = dir;
  Thrust::vec = glm::vec2(cos((Position::apos-90.0f) * 6.28f/360.0f), 
                          sin((Position::apos-90.0f) * 6.28f/360.0f)) * glm::length(Thrust::vec);
}

void 
Ship::stopThrust() {
  Thrust::vec = glm::vec2(0.0f);
  getRenderPass().pTex[0] = pRoidRage->pSpaceshipTex[1].get();
}

void 
Ship::stopAttitude() {
  Attitude::dir = 0.0f;
  Position::avel = 0.0f;
}

//------------------------------------------------------------------------------

void 
Ship::shoot() {
  //if (!pewpewLimiter_.consume()) {
  //  return;
  //}

  //pRoidRage->pSfxPlayer->pew();

  shoot(0.0f);

  //if (hasTriple_) {
    shoot( 15.0f);
    shoot(-15.0f);
  //}
}

//------------------------------------------------------------------------------
// Factor out 
void 
Ship::shoot(float angle) {

  // Create a new pewpew
  PewPew* pPewPew = new PewPew(Position::pos.x, Position::pos.y, PewPew::SHIP);
  pPewPew->Position::vel = Position::vel + (glm::vec2(cos((Position::apos-90.0f-angle) * 6.28f/360.0f), 
                                                      sin((Position::apos-90.0f-angle) * 6.28f/360.0f)) * 3.0f);
  // Powerups!
  //if (hasCharge_) {
    pPewPew->supercharge();
  //}

  // Add this pewpew to the population
  pRoidRage->population.add(pPewPew);
}

//------------------------------------------------------------------------------

void
Ship::charge() {
  hasCharge_ = true;
  chargeAgeMs_ = 0;
}

//------------------------------------------------------------------------------

void
Ship::triple() {
  hasTriple_ = true;
  tripleAgeMs_ = 0;
}

//------------------------------------------------------------------------------

void
Ship::shield(int durationMs) {
  if (hasShield_) return; 

  hasShield_ = true;
  shieldAgeMs_ = durationMs;
}

//------------------------------------------------------------------------------

void
Ship::ageCharge() {
  if (hasCharge_) {
    chargeAgeMs_ += TimeDelta::ms;
    if (chargeAgeMs_ > chargeExpirationMs_ ) {
      hasCharge_ = false;
      chargeAgeMs_ = 0;
    }
  }
}

//------------------------------------------------------------------------------

void
Ship::ageTriple() {
  if (hasTriple_) {
    tripleAgeMs_ += TimeDelta::ms;
    if (tripleAgeMs_ > tripleExpirationMs_ ) {
      hasTriple_ = false;
      tripleAgeMs_ = 0;
    }
  }
}

//------------------------------------------------------------------------------

void
Ship::ageShield() {
  if (hasShield_) {
    shieldAgeMs_ -= TimeDelta::ms;
    if (shieldAgeMs_ < 0) {
      hasShield_ = false;
      shieldAgeMs_ = 0;
    }
  }
}


//------------------------------------------------------------------------------

void 
Ship::onCollideWith(Ship* pShip, Collision* pCollision) {
}

//------------------------------------------------------------------------------

void 
Ship::onCollideWith(Roid* pRoid, Collision* pCollision) {
  VALIDATE(pRoid)
  VALIDATE(this)

  glm::vec2 normal       = glm::normalize(pCollision->displacement);

  Position::vel  = glm::reflect(Position::vel, normal) * 0.7f;
  Position::pos += pCollision->displacement; 

  //pRoidRage->pSfxPlayer->pew(200);
  takeDamage(0.3f);
  shield();
}

//------------------------------------------------------------------------------

void 
Ship::onCollideWith(PewPew* pPewPew, Collision* pCollision) {
  VALIDATE(pPewPew)
  VALIDATE(this)

  // No friendly fire
  if (pPewPew->getType() == PewPew::SHIP) {
    return;
  }

  //pRoidRage->pSfxPlayer->pew(200);
  takeDamage(0.2f);
  pPewPew->invalidate();
}

//------------------------------------------------------------------------------

void 
Ship::onCollideWith(Bonus* pBonus, Collision* pCollision) {
}

//------------------------------------------------------------------------------

void 
Ship::onRender(OrthoCamera* pCam) {
  Quad::onRender(pCam);

  /*
  LinearIndicator shield(0.0f, 15.0f);
  shield.update(Shield::mag); 
  //shield.getRenderPass().modelMatrix = getRenderPass().modelMatrix;
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
  */

  /*
  if (pRoidRage->pShip->hasShield()) {
    float pulse = 0.5f * sin(2.0f * M_PI * 1.0f * pRoidRage->gameTime/1000.0f);


    ShadedQuad bg(-100.0f - pulse        * pCam->getZoom()*0.5f, 
                  -100.0f - pulse        * pCam->getZoom()*0.5f, 
                   200.0f + pulse * 2.0f * pCam->getZoom(), 
                   200.0f + pulse * 2.0f * pCam->getZoom(), 
                   pRoidRage->pShieldProgram.get());

    ShadedQuad bg(-5.0f - pulse       , 
                  -5.0f - pulse       , 
                   10.0f + pulse * 2.0f, 
                   10.0f + pulse * 2.0f, 
                   pRoidRage->pShieldProgram.get());
    
    bg.getRenderPass().color = glm::vec4(0.5f, 0.5f, 0.8f, 0.3f);
    bg.getRenderPass().modelMatrix = getRenderPass().modelMatrix * bg.getRenderPass().modelMatrix;
    bg.render(pCam);
  }
  */
}

//------------------------------------------------------------------------------

void
Ship::takeDamage(float damage) {
  if (hasShield_) return;

  // Shields can absorb the damage
  Shield::mag -= damage;  

  // If the shield is depleted, take a life
  if (Shield::mag < 0.02f) { 
    pRoidRage->lives--;
    Shield::mag = 1.0f;

    Text* pText = new Text(pos.x, pos.y);
    pText->expiresInMs = 3000;
    pText->setText("crunch");
    pRoidRage->population.add(pText);
    //pRoidRage->pSfxPlayer->pew(60);
  } 

  Log::info("Damaging shield[%]: %", damage, Shield::mag);

  // If we're out of lives, invalidate the ship
  if (pRoidRage->lives <= 0) {
    invalidate();
  }
}

//------------------------------------------------------------------------------

bool
Ship::hasShield() const {
  return hasShield_;
}

//------------------------------------------------------------------------------
