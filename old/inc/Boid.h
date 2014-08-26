//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_BOID_H
#define INCLUDED_BOID_H

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
struct PewPew;

//------------------------------------------------------------------------------

struct Boid : public Quad
            , public TimeDelta
            , public TimeMs
            , public Position
            , public Charge
            , public Mass
            , public Glow
            , public Radius
            , public Thrust
            , public Validate
            , public pronghorn::CollisionPair<pronghorn::This<Boid>, Ship,   Collision*>
            , public pronghorn::CollisionPair<pronghorn::This<Boid>, PewPew, Collision*>
{
public:
  Boid(float x, float y);
  virtual ~Boid();

  void updateThrust(const glm::vec2& acc);
  void stopThrust();

  void shoot();
  void shoot(float angle);

  int  getSwarm() const;

protected:
  void onCollideWith(Ship*   pShip,   Collision* pCollision);
  void onCollideWith(Boid*   pBoid,   Collision* pCollision);
  void onCollideWith(PewPew* pPewPew, Collision* pCollision);

private:
  int64_t timeSinceLastPewMs_;  
  int     swarm_;

public:
  static void reset();
  static void calcAverage();

  // TODO Factor out flocking
  static const unsigned  numSwarms = 3;
  static glm::vec2 avgPos[numSwarms];
  static glm::vec2 avgVel[numSwarms];
  static unsigned  count[numSwarms];
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
