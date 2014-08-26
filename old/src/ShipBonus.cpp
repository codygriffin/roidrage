//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "ShipBonus.h"
#include "Ship.h"

#include "RoidRage.h"

//------------------------------------------------------------------------------

using namespace roidrage;
using namespace pronghorn;

//------------------------------------------------------------------------------

ShipBonus::ShipBonus(float x, float y) 
  : Bonus(x, y) {
  Radius::mag    = 100.0f;
  Position::avel = 0.0f;
  getRenderPass().pTex[0] = pRoidRage->pBonusTex[3].get();
}

//------------------------------------------------------------------------------

ShipBonus::~ShipBonus() {
}

//------------------------------------------------------------------------------

void 
ShipBonus::onUse(Ship* pShip) {
  pRoidRage->lives++;
  points_=0;
}

//------------------------------------------------------------------------------
