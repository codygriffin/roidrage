//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_SHIP_H
#define INCLUDED_SHIP_H

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

struct Roid;
struct Boid;
struct Bonus;
struct PewPew;

//------------------------------------------------------------------------------

struct Ship : public Quad
            , public TimeDelta
            , public TimeMs
            , public Position
            , public Charge
            , public Mass
            , public Radius
            , public Thrust
            , public Shield
            , public Fuel
            , public Validate
            , public pronghorn::CollisionPair<pronghorn::This<Ship>, Roid,   Collision*>
            , public pronghorn::CollisionPair<pronghorn::This<Ship>, Bonus,  Collision*>
            , public pronghorn::CollisionPair<pronghorn::This<Ship>, PewPew, Collision*>
{
public:
  Ship();
  virtual ~Ship();

  void updateThrust(const glm::vec2& acc);
  void stopThrust();

  void shoot();
  void shoot(float angle);

  // Powerups!
  void charge();
  void triple();
  void shield(int durationMs = 500);

  void ageCharge();
  void ageTriple();
  void ageShield();

  bool hasShield() const;

protected:
  void onCollideWith(Ship*   pShip,   Collision* pCollision);
  void onCollideWith(Roid*   pRoid,   Collision* pCollision);
  void onCollideWith(Boid*   pBoid,   Collision* pCollision);
  void onCollideWith(Bonus*  pBonus,  Collision* pCollision);
  void onCollideWith(PewPew* pPewPew, Collision* pCollision);

  void onRender(OrthoCamera* pCam);

public:
  unsigned points_;
  bool     hasCharge_;
  bool     hasTriple_;
  bool     hasShield_;

  unsigned chargeAgeMs_;
  unsigned chargeExpirationMs_;

  unsigned tripleAgeMs_;
  unsigned tripleExpirationMs_;

  int      shieldAgeMs_;

  RateLimiter pewpewLimiter_;

private:
  void takeDamage(float damage);

};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
