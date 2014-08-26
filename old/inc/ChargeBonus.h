//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_CHARGEBONUS_H
#define INCLUDED_CHARGEBONUS_H

#ifndef INCLUDED_BONUS_H
#include "Bonus.h"
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct ChargeBonus : public Bonus {
  ChargeBonus(float x, float y);
  virtual ~ChargeBonus();

protected:
	virtual void onUse(Ship* pShip);

private:

};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
