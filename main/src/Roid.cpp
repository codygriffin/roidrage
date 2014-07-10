//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Roid.h"
#include "Ship.h"
#include "PewPew.h"
#include "Text.h"
#include "Bonus.h"
#include "TripleBonus.h"
#include "ChargeBonus.h"
#include "ShipBonus.h"

#include "RoidRage.h"

#include "Log.h"
#include "Population.h"
#include "AudioState.h"
#include "Collision.h"

//#define HYPERSPACE

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace roidrage;

//------------------------------------------------------------------------------

Roid::Roid(float x, float y)
: Quad         (pRoidRage->pRoidTex[rand()%3].get(), pRoidRage->pProgram.get(), pRoidRage->pQuadVboPos.get())
, TimeDelta    ()
, TimeMs       (TimeMs::now())
, Position     ()
, Mass         (0.0f)
, Radius       (200.0f + (rand() % 300))
, Charge       (-300.0f)
{
  Position::pos  = glm::vec2(x, y);
  Position::avel = ((rand()%100) - 50)/500.0f;
  Position::vel  = glm::normalize(glm::vec2(rand()%100-50, rand()%100-50)) * 0.2f; 

  Mass::mag   = Radius::mag * Radius::mag;
  Charge::mag = -30000.0f;
}

//------------------------------------------------------------------------------

Roid::~Roid() {
}

//------------------------------------------------------------------------------

void 
Roid::onCollideWith(PewPew* pPewPew, Collision* pCollision) {
  // XXX
  VALIDATE(pPewPew)
  VALIDATE(this)

  if (Radius::mag > 230.0f && pRoidRage->population.getRelation<Roid*>().count() < 100) {
    // Break up Roid
    Roid* pRoid1 = new Roid(Position::pos.x, Position::pos.y);
    pRoid1->Position::vel = Position::vel + glm::normalize(glm::vec2(rand()%100, rand()%100)) * 0.3f; 
    pRoid1->Radius::mag = Radius::mag*0.7f;
    pRoid1->Mass::mag = Mass::mag/3.0f;
    pRoid1->Charge::mag = pRoid1->Charge::mag;

    Roid* pRoid2 = new Roid(Position::pos.x, Position::pos.y);
    pRoid2->Position::vel = -Position::vel + glm::normalize(glm::vec2(rand()%100, rand()%100)) * 0.3f; 
    pRoid2->Radius::mag = Radius::mag*0.7f;
    pRoid2->Mass::mag = Mass::mag/3.0f;
    pRoid2->Charge::mag = pRoid2->Charge::mag;

    Roid* pRoid3 = new Roid(Position::pos.x, Position::pos.y);
    pRoid3->Position::vel = -Position::vel + glm::normalize(glm::vec2(rand()%100, rand()%100)) * 0.3f; 
    pRoid3->Radius::mag = Radius::mag*0.7f;
    pRoid3->Mass::mag = Mass::mag/3.0f;
    pRoid3->Charge::mag = pRoid3->Charge::mag;

    pRoidRage->population.add(pRoid1);
    pRoidRage->population.add(pRoid2);
    pRoidRage->population.add(pRoid3);
  }
  else if (pPewPew->getType() == PewPew::SHIP) {
    // Roid is destroyed, so we give up some Bonus
    if (rand()%1000 < 2) { // 0.2%
      Bonus* pBonus = new ShipBonus(Position::pos.x, Position::pos.y);
      pRoidRage->population.add(pBonus);
    } else if (rand()%1000 < 10) {  // 1.0%
      Bonus* pBonus = new TripleBonus(Position::pos.x, Position::pos.y);
      pRoidRage->population.add(pBonus);
    } else if (rand()%1000 < 10) { // 1.0%
      Bonus* pBonus = new ChargeBonus(Position::pos.x, Position::pos.y);
      pRoidRage->population.add(pBonus);
    } else if (rand()%100 < 10) { // 10%
      Bonus* pBonus = new Bonus(Position::pos.x, Position::pos.y);
      pRoidRage->population.add(pBonus);
    }
  }

  // TODO Only player gets points
  if (pPewPew->getType() == PewPew::SHIP) {
    const unsigned points = 100;
    pRoidRage->score += points;

    // Render Text
    Text* pText = new Text(pos.x, pos.y);
    pText->Position::vel = this->vel;
    pText->setText("%", points);
    pRoidRage->population.add(pText);
  }

  //pRoidRage->pSfxPlayer->pew(150);

  invalidate();
  pPewPew->invalidate();
}

//------------------------------------------------------------------------------

void 
Roid::onCollideWith(Ship* pShip, Collision* pCollision) {
  // From the Roid's perspective, nothing to do here.
}

//------------------------------------------------------------------------------

void 
Roid::onCollideWith(Roid* pRoid, Collision* pCollision) {
  // From the Roid's perspective, nothing to do here.
}

//------------------------------------------------------------------------------
