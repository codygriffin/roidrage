//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "TripleBonus.h"
#include "Ship.h"

#include "RoidRage.h"

//------------------------------------------------------------------------------

using namespace roidrage;
using namespace pronghorn;

//------------------------------------------------------------------------------

TripleBonus::TripleBonus(float x, float y) 
  : Bonus(x, y) {

  Radius::mag    = 100.0f;
  Position::avel = 0.0f;
  getRenderPass().pTex[0] = pRoidRage->pBonusTex[2].get();
}

//------------------------------------------------------------------------------

TripleBonus::~TripleBonus() {
}

//------------------------------------------------------------------------------

void 
TripleBonus::onUse(Ship* pShip) {
  pShip->triple();
  points_=0;
}

//------------------------------------------------------------------------------
