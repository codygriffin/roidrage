//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "PewPew.h"
#include "RoidRage.h"

#include "OrthoCamera.h"
#include "Collision.h"

#include "Log.h"

using namespace pronghorn;
using namespace roidrage;

//------------------------------------------------------------------------------

PewPew::PewPew(float x, float y, Type type)
: Quad       (pRoidRage->pPewPewTex[0].get(), pRoidRage->pProgram.get(), pRoidRage->pQuadVboPos.get())
, TimeDelta  ()
, TimeMs     (TimeMs::now())
, Position   ()
, Radius     (30.0f)
, Mass       (0.01f)
, Charge     (0.0f)
, isDud_     (false)
, type_      (type)
{
  Position::pos    = glm::vec2(x, y);
  Position::maxVel = 5.0f;
  Position::avel   = 0.1f;

  if (type_ == BOID) {
    getRenderPass().pTex[0] = pRoidRage->pPewPewTex[2].get();
    Radius::mag = 20.0f;
    distance+=4000.0f;
  }
}

//------------------------------------------------------------------------------

PewPew::~PewPew() {
}

//------------------------------------------------------------------------------

void 
PewPew::supercharge() {
  Charge::mag = 80.0f;
  getRenderPass().pTex[0] = pRoidRage->pPewPewTex[1].get();
}

//------------------------------------------------------------------------------

void 
PewPew::dud() {
  isDud_ = true;
}

//------------------------------------------------------------------------------

void 
PewPew::onCollideWith(Roid* pRoid, Collision* pCollision) {
}

//------------------------------------------------------------------------------

void 
PewPew::onCollideWith(Boid* pBoid, Collision* pCollision) {
}

//------------------------------------------------------------------------------

void 
PewPew::onCollideWith(PewPew* pPewPew, Collision* pCollision) {
}

//------------------------------------------------------------------------------

void 
PewPew::onCollideWith(Ship* pShip, Collision* pCollision) {
}

//------------------------------------------------------------------------------

bool 
PewPew::isDud() const {
  return isDud_;  
}

//------------------------------------------------------------------------------

PewPew::Type 
PewPew::getType() const {
  return type_;  
}

//------------------------------------------------------------------------------
