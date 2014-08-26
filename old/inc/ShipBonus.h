//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_SHIPBONUS_H
#define INCLUDED_SHIPBONUS_H

#ifndef INCLUDED_BONUS_H
#include "Bonus.h"
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct ShipBonus : public Bonus {
  ShipBonus(float x, float y);
  virtual ~ShipBonus();

protected:
	virtual void onUse(Ship* pShip);

private:

};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
