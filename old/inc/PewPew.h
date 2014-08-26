//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_PEWPEW_H
#define INCLUDED_PEWPEW_H

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

#include "Collision.h"

//------------------------------------------------------------------------------

namespace roidrage { 

struct Ship;
struct Roid;
struct Boid;

//------------------------------------------------------------------------------

struct PewPew : public Quad
              , public TimeDelta
              , public TimeMs
              , public Position
              , public Radius 
              , public Mass 
              , public Charge
              , public Glow 
              , public Validate 
              , public pronghorn::CollisionPair<pronghorn::This<PewPew>, Roid, Collision*>
              , public pronghorn::CollisionPair<pronghorn::This<PewPew>, Boid, Collision*>
              , public pronghorn::CollisionPair<pronghorn::This<PewPew>, Ship, Collision*>
{
  enum Type {
    SHIP = 0,
    BOID = 1
  };

  PewPew(float x, float y, Type type = SHIP);
  virtual ~PewPew();

  void supercharge();
  void dud();

  bool isDud() const;
  Type getType() const;

protected:
  void onCollideWith(Ship*   pShip,   Collision* pCollision);
  void onCollideWith(Roid*   pRoid,   Collision* pCollision);
  void onCollideWith(Boid*   pBoid,   Collision* pCollision);
  void onCollideWith(PewPew* pPewPew, Collision* pCollision);

private:
  bool isDud_;
  Type type_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
