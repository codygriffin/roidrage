//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Bonus.h"
#include "Ship.h"
#include "PewPew.h"
#include "Text.h"

#include "RoidRage.h"

#include "Log.h"
#include "Population.h"
#include "Collision.h"

using namespace roidrage;
using namespace pronghorn;

//------------------------------------------------------------------------------

Bonus::Bonus(float x, float y)
: Quad          (pRoidRage->pBonusTex[0].get(), pRoidRage->pProgram.get(), pRoidRage->pQuadVboPos.get())
, TimeDelta     ()
, TimeMs        (TimeMs::now())
, Position      ()
, Mass          (0.0f)
, Charge        (0.1f)
, Radius        (60.0f)
, Expiration    (8000)
, points_       (500)
{
  Position::pos  = glm::vec2(x, y);
  Position::avel = 0.1f;
}

//------------------------------------------------------------------------------

Bonus::~Bonus() {
}

//------------------------------------------------------------------------------

void 
Bonus::use(Ship* pShip) {
  onUse(pShip);
}

//------------------------------------------------------------------------------

void 
Bonus::onCollideWith(Ship* pShip, Collision* pCollision) {
  VALIDATE(pShip)
  VALIDATE(this)

  use(pShip);
}

//------------------------------------------------------------------------------

void 
Bonus::onCollideWith(Bonus* pBonus, Collision* pCollision) {
}

//------------------------------------------------------------------------------

void 
Bonus::onUse(Ship* pShip) {
  if (points_) {
    Text* pText = new Text(pos.x, pos.y);
    pText->setText("%", points_);
    pText->Position::vel = this->vel;
    pRoidRage->population.add(pText);

    pRoidRage->score += points_;
    points_ = 0; //Use up points
  }
  invalidate();
}

//------------------------------------------------------------------------------
