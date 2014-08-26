//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_BONUS_H
#define INCLUDED_BONUS_H

#ifndef INCLUDED_OPTIONAL_H
#include "TimeMs.h"
#endif

#ifndef INCLUDED_RENDERABLE_H
#include "Renderable.h"
#endif

#ifndef INCLUDED_COLLISION_H
#include "Collision.h"
#endif

#ifndef INCLUDED_PARAMS_H
#include "Params.h"
#endif

#include "Validate.h"

//------------------------------------------------------------------------------

namespace roidrage { 

struct Ship;

//------------------------------------------------------------------------------

struct Bonus : public Quad
             , public TimeDelta
             , public TimeMs
             , public Position
             , public Radius 
             , public Mass 
             , public Charge 
             , public Expiration 
             , public Validate 
             , public pronghorn::CollisionPair<pronghorn::This<Bonus>, Ship, Collision*>
{
  Bonus(float x, float y);
  virtual ~Bonus();

	void use(Ship* pShip);

protected:
  void onCollideWith(Ship* pShip, Collision* pCollision);
  void onCollideWith(Bonus* pBonus, Collision* pCollision);

	virtual void onUse(Ship* pShip);

public://XXX
  unsigned points_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
