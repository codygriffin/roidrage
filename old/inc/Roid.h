//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_ROID_H
#define INCLUDED_ROID_H

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
struct Bonus;
struct PewPew;

//------------------------------------------------------------------------------

struct Roid : public Quad
            , public TimeDelta 
            , public TimeMs
            , public Position 
            , public Mass
            , public Radius 
            , public Charge
            , public Validate
            , public pronghorn::CollisionPair<pronghorn::This<Roid>, Ship, Collision*>
            , public pronghorn::CollisionPair<pronghorn::This<Roid>, PewPew, Collision*>
{
public:
  Roid(float x, float y);
  virtual ~Roid();

protected:
  void onCollideWith(PewPew* pPewPew, Collision* pCollision);
  void onCollideWith(Ship* pShip, Collision* pCollision);
  void onCollideWith(Roid* pRoid, Collision* pCollision);

private:
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
