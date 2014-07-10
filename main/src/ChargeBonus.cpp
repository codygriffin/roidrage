//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "ChargeBonus.h"
#include "Ship.h"

#include "RoidRage.h"

//------------------------------------------------------------------------------

using namespace roidrage;
using namespace pronghorn;

//------------------------------------------------------------------------------

ChargeBonus::ChargeBonus(float x, float y) 
  : Bonus(x, y) {
  Radius::mag    = 100.0f;
  Position::avel = 0.0f;
  getRenderPass().pTex[0] = pRoidRage->pBonusTex[1].get();
}

//------------------------------------------------------------------------------

ChargeBonus::~ChargeBonus() {
}

//------------------------------------------------------------------------------

void 
ChargeBonus::onUse(Ship* pShip) {
  pShip->charge();
  points_=0;
}

//------------------------------------------------------------------------------
