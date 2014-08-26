//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_TRIPLEBONUS_H
#define INCLUDED_TRIPLEBONUS_H

#ifndef INCLUDED_BONUS_H
#include "Bonus.h"
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct TripleBonus : public Bonus  {
  TripleBonus(float x, float y);
  virtual ~TripleBonus();

protected:
	virtual void onUse(Ship* pShip);

private:
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
