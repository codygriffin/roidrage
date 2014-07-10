//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_SYSTEMS_H
#define INCLUDED_SYSTEMS_H

#ifndef INCLUDED_ORTHOCAMERA_H
#include "OrthoCamera.h"
#endif

#ifndef INCLUDED_COLLISION_H
#include "Collision.h"
#endif

#ifndef INCLUDED_PARAMS_H
#include "Params.h"
#endif

#ifndef INCLUDED_RENDERSTATE_H
#include "RenderState.h"
#endif

#ifndef INCLUDED_RENDERABLE_H
#include "Renderable.h"
#endif

#ifndef INCLUDED_BACKGROUND_H
#include "Background.h"
#endif

#ifndef INCLUDED_TEXT_H
#include "Text.h"
#endif

#ifndef INCLUDED_PEWPEW_H
#include "PewPew.h"
#endif

#ifndef INCLUDED_BONUS_H
#include "Bonus.h"
#endif

#ifndef INCLUDED_BOID_H
#include "Boid.h"
#endif

//------------------------------------------------------------------------------

namespace roidrage { 

extern "C" {
  void renderOrtho(Renderable* pRenderable);
  void renderOverlay(Renderable* pRenderable);
  void renderGlow(Renderable* pRenderable, Glow* pGlow);
  void renderChar(RenderPass* pass, char c, float x, float y, float r);
  void renderFrame(RenderPass* pass, float width, float height, float scale);

  void updateTime(TimeMs* pTime, TimeDelta* pDelta);
  void resetAcceleration(Position* pPos);
  void updateThrust(Position* pPos, Thrust* pThrust);
  void updatePosition(TimeDelta* delta, Position* pos);
  void updateRoidar(Roid* pRoid);
  void age(TimeDelta* pDelta, Expiration* pExpiration);
  void expiredBonuses(Bonus* pBonus);
  void expirePewPews(PewPew* pPewPew);
  void expiredText(Text* pText);
  void expirePowerups(Ship* pShip);
  void regenerateShield(TimeDelta* pDelta, Shield* pShield);

  void updateTransform(Renderable* pRenderable, Position* p, Radius* m);
  void updateAcceleration(Position* p1,   
                          Charge* c1, 
                          Mass* m1,
                          Position* p2, 
                          Charge* c2, 
                          Mass* m2);
  void updateBoidAverages(Boid* pBoid);
  void updateBoidPosition(Boid* pBoid);
  void updateBoidAvoidance(Boid* pBoid1, Boid* pBoid2);
};

//------------------------------------------------------------------------------

template <typename A, typename B>
inline void updateCollision(pronghorn::CollisionPair<A, B, Collision*>* b1,
                     Position*                     p1,
                     Radius*                       r1,
                     pronghorn::CollisionPair<A, B, Collision*>* b2,
                     Position*                     p2,
                     Radius*                       r2) {

  // Obviously we don't collide with ourselves
  if (b1 == b2) return;

  // Boundary is the minimum allowable distance 
  // between colliding entities
  auto boundary = r1->mag + r2->mag;

  // Displacement is the vector pointing from this entity to the 
  // other entity, with a length corresponding to the distance
  auto displacement = p1->pos - p2->pos;

  if (boundary > glm::length(displacement)) { 
    // Really, we only care about overlap in the collision
    displacement = glm::normalize(displacement) * 
                  (boundary - glm::length(displacement));
    Collision collision(displacement, boundary);
    b2->onCollideWith(b1, &collision);
  }
}

//------------------------------------------------------------------------------

inline void enforceBoundaries(Position* p1, Radius* r1) {

  if (p1->pos.x + r1->mag >  Position::max && p1->vel.x > 0) {
    p1->pos.x = Position::max - r1->mag;
    p1->vel.x = -p1->vel.x; 
  }

  if (p1->pos.y + r1->mag >  Position::max && p1->vel.y > 0) {
    p1->pos.y = Position::max - r1->mag;
    p1->vel.y = -p1->vel.y;
  }

  if (p1->pos.x - r1->mag < -Position::max && p1->vel.x < 0) {
    p1->pos.x = -Position::max + r1->mag;
    p1->vel.x = -p1->vel.x;
  }

  if (p1->pos.y - r1->mag < -Position::max && p1->vel.y < 0) {
    p1->pos.y = -Position::max + r1->mag;
    p1->vel.y = -p1->vel.y;
  }
}

//------------------------------------------------------------------------------

template<class... T>
void 
renderTextOverlay(float x,  float y, 
                  float r, 
                  const char* msg, 
                  T... args) {
  Text quad(x,y); 
  const float offsetx = r;
  const float offsety = r;
  const float padx    = 0.0f;
  const float pady    = r/2.0f;
  std::stringstream stream;
  toStream(stream, msg, args...);
  auto text = stream.str();
  float x_ = x; 
  for (auto i  = 0; i < text.length(); i++) {
    if (text[i] == '\n') {
      y += r + pady; 
      x_ = x;
    } else {
      renderChar(&quad.getRenderPass(), text[i], offsetx + x_, offsety + y, r);
      x_ += r + padx;
    }
  }
}

template<class... T>
void 
renderText(RenderPass* pPass, 
           float x,  float y, 
           float r, 
           const char* msg, 
           T... args) {
  const float offsetx = r;
  const float offsety = r;
  const float padx    = 0.0f;
  const float pady    = r/2.0f;
  std::stringstream stream;
  toStream(stream, msg, args...);
  std::string text = stream.str();
  float x_ = x; 
  for (size_t i  = 0; i < text.length(); i++) {
    if (text[i] == '\n') {
      y += r + pady; 
      x_ = x;
    } else {
      renderChar(pPass, text[i],  offsetx + x_, offsety + y, r);
      x_ += r + padx;
    }
  }
}

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
