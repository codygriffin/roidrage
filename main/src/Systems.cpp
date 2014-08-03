//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Systems.h"
#include "RoidRage.h"
#include "PewPew.h"
#include "Ship.h"
#include "Bonus.h"
#include "Roid.h"
#include "Text.h"
#include "Background.h"

#include "Log.h"

//------------------------------------------------------------------------------

using namespace roidrage;
using namespace pronghorn;

constexpr float Thrust::unit;
constexpr float Attitude::unit;
constexpr float Position::max;
constexpr float Mass::unit;
constexpr float Charge::unit;

//------------------------------------------------------------------------------

extern "C" {

//------------------------------------------------------------------------------

void 
updateTime(TimeMs* pTime, TimeDelta* pDelta) {
  pDelta->ms = pTime->mark();
}

//------------------------------------------------------------------------------

void 
resetAcceleration(Position* pPos) {
  pPos->acc  = glm::vec2(0.0f);
  pPos->aacc = 0.0f;
}

//------------------------------------------------------------------------------

void updateThrust(Position* pPos, Thrust* pThrust, Attitude* pAttitude) {
  pPos->acc  += pThrust->vec * Thrust::unit;
  pPos->aacc += pAttitude->dir * Attitude::unit;
  pThrust->vec = glm::vec2(cos((pPos->apos-90.0f) * 6.28f/360.0f),
                           sin((pPos->apos-90.0f) * 6.28f/360.0f)) 
               * -glm::length(pThrust->vec);
}

//------------------------------------------------------------------------------

void updatePosition(TimeDelta* delta, Position* pos) {
  // Clamp that shit down - what about lightspeed/relativity?
  if (glm::length(pos->vel) > pos->maxVel) {
    pos->vel = glm::normalize(pos->vel) * pos->maxVel;
  }

  //TODO better integration (RK4?)
  pos->vel     += pos->acc     * float(delta->ms);
  pos->avel    += pos->aacc    * float(delta->ms);

  pos->pos     += pos->vel     * float(delta->ms);
  pos->apos    += pos->avel    * float(delta->ms);


  // Clamp our angulars
  if (pos->apos >  360.0f) {
    pos->apos = 0.0f;
  }
  if (pos->apos < -360.0f) {
    pos->apos = 0.0f;
  }

  if (pos->avel >  3.0f) {
    pos->avel = 3.0f;
  }
  if (pos->avel < -3.0f) {
    pos->avel = -3.0f;
  }

  if (pos->aacc >  0.004f) {
    pos->aacc = 0.004f;
  }
  if (pos->aacc < -0.004f) {
    pos->aacc = -0.004f;
  }

  // scalar distance - used for expiring pewpews (perhaps time is better?)
  pos->distance += glm::length(pos->vel * float(delta->ms));
}

//------------------------------------------------------------------------------

void age(TimeDelta* pDelta, Expiration* pExpiration) {
  pExpiration->age(pDelta->ms);
}

//------------------------------------------------------------------------------

void expirePewPews(PewPew* pPewPew) {
  if (pPewPew->Position::distance >= Position::maxDistance) {
    pPewPew->invalidate();
  }
}

//------------------------------------------------------------------------------

// TODO consolidate expiration logic
void expiredBonuses(Bonus* pBonus) {
  if (pBonus->isExpired() || !pBonus->points_) {
    pBonus->invalidate();
  }
}

//------------------------------------------------------------------------------

void 
expiredText(Text* pText) {
  if (pText->isExpired()) {
    pText->invalidate();
  }
}

//------------------------------------------------------------------------------

void 
expirePowerups(Ship* pShip) {
  pShip->ageCharge();
  pShip->ageTriple();
  pShip->ageShield();
}

//------------------------------------------------------------------------------

void 
regenerateShield(TimeDelta* pDelta, Shield* pShield) {
  pShield->mag += pDelta->ms * 0.00008f;
  if (pShield->mag > 1.0f) {
    pShield->mag = 1.0f;
  }
}

//------------------------------------------------------------------------------

void 
updateTransform(Renderable* pRenderable, Position* p, Radius* m) {
  glm::vec2 pos = p->pos;
  float     mag = m->mag;

  // Set our position, rotation and scale according
  // to the position, radius and angular position.
  pRenderable->getRenderPass().modelMatrix = glm::scale(
                        glm::translate(
                          glm::mat4(),
                          glm::vec3(pos.x, pos.y, 0.0f)
                        ),
                        glm::vec3(mag*1.0f)
                      );

  pRenderable->getRenderPass().modelMatrix = glm::rotate(
                        pRenderable->getRenderPass().modelMatrix, 
                        glm::radians(p->apos),
                        glm::vec3(0.0f,0.0f,1.0f)
                      );
}

//------------------------------------------------------------------------------

void updateAcceleration(Position* p1,   
                        Charge* c1, 
                        Mass* m1,
                        Position* p2, 
                        Charge* c2, 
                        Mass* m2) {
  if (p1 == p2) return;

  glm::vec2 diff = p2->pos - p1->pos;
  float r = glm::length(diff);

  // Clamp to sane values
  if (r <  10.0f)          return;
  if (r >  1500.0f)        return;
  if (m1->mag < 0.00001f)  return;


  const float mr2 = m1->mag*r*r;
  glm::normalize(diff);

  // TODO Really there are two forces packed in here
  glm::vec2 acc  = diff *   Mass::unit*m1->mag*m2->mag / mr2;
  acc           += diff * Charge::unit*c1->mag*c2->mag / mr2;

  p1->acc += acc;
  if (p1->acc.x != p1->acc.x && p1->acc.y != p1->acc.y) {
    Log::error("Acceleration is NaN after update");
    p1->acc = glm::vec2(0.0f, 0.0f);
  }
}

//------------------------------------------------------------------------------

#define DUMP_VEC2(n,v) Log::debug(#n "(%, %)", v.x, v.y);

void updateBoidAverages(Boid* pBoid) {
  Boid::avgPos[pBoid->getSwarm()] += pBoid->Position::pos;
  Boid::avgVel[pBoid->getSwarm()] += pBoid->Position::vel;
  Boid::count[pBoid->getSwarm()]++;
}

//------------------------------------------------------------------------------

// Tend to move towards player and group average (flocking/hunting)
void updateBoidPosition(Boid* pBoid) {
  glm::vec2 dir = glm::normalize(pBoid->Position::vel);
  glm::vec2 dP  = Boid::avgPos[pBoid->getSwarm()] - pBoid->Position::pos;
  glm::vec2 dV  = Boid::avgVel[pBoid->getSwarm()] - dir;
  glm::vec2 dS  = pRoidRage->pShip->Position::pos - pBoid->Position::pos;

  if (Boid::count[pBoid->getSwarm()] == 1 && glm::length(dS) != 0.0f) {
    const float hunt      = 0.000001f;
    pBoid->acc            = dS * hunt;
    pBoid->Position::apos = atan2(pBoid->vel.y, pBoid->vel.x) * 360.0f/6.28f + 90.0f;

    const float distance = glm::length(dS);
    if (distance > 0) {
      dS = glm::normalize(dS);
      float gaze = glm::length(dir - dS);
      if (gaze < 0.50f && distance < 3000.0f) {
        pBoid->shoot();
      }
    }

    return;
  }
  
  if (glm::length(dP) == 0.0f ||
      glm::length(dV) == 0.0f ||
      glm::length(dS) == 0.0f) {
    return;
  }

  const float distance = glm::length(dS);

  const float hunt      = 0.0008f;
  const float cohesion  = 0.0001f;
  const float alignment = 0.00001f;

  dP = glm::normalize(dP);
  dS = glm::normalize(dS);
  dV = glm::normalize(dV);

  float gaze = glm::length(dir - dS);
  if (gaze < 0.40f && distance < 2500.0f) {
    pBoid->shoot();
  }

  if (Boid::count[pBoid->getSwarm()] > 2) {
    pBoid->acc += dP * cohesion;
    pBoid->acc += dP * alignment;
  }

  pBoid->acc += dS * hunt;

  pBoid->Position::apos = atan2(pBoid->vel.y, pBoid->vel.x) * 360.0f/6.28f + 90.0f;
}

void updateBoidAvoidance(Boid* pBoid1, Boid* pBoid2) {
  if (pBoid1 == pBoid2) return;
  
  const float separation    = (pBoid1->getSwarm() != pBoid2->getSwarm()) ? 0.0008f : 0.004f;
  const float purpleCircle  = 1.7f;

  // Boundary is the minimum distance between colliding entities
  auto boundary = (pBoid1->Radius::mag + pBoid2->Radius::mag) * purpleCircle;

  auto dB = pBoid1->pos - pBoid2->pos;

  if (boundary > glm::length(dB) && glm::length(dB) != 0) { 
    dB = glm::normalize(dB);
    pBoid1->acc += dB * separation;
  }
}

//------------------------------------------------------------------------------

void renderOverlay(Renderable* pRenderable) {
  pRenderable->render(RenderState::pCam_.get());
}

//------------------------------------------------------------------------------

void renderOrtho(Renderable* pRenderable) {
  pRenderable->render(RenderState::pCam_.get());
}

//------------------------------------------------------------------------------

void renderGlow(Renderable* pRenderable, Glow* pGlow) {
  pRenderable->render(RenderState::pCam_.get());
}

//------------------------------------------------------------------------------

void renderChar(RenderPass* pass, char c, float x, float y, float r) {
  pass->modelMatrix = glm::scale(
                        glm::translate(
                          glm::mat4(),
                          glm::vec3(x, y, 0.0f)
                        ),
                        glm::vec3(r)
                      );

  pass->pProgram->use();

  int nx = c % 16;
  int ny = c / 16;

  pass->pProgram->uniform  ("offsetX", (float)nx);
  pass->pProgram->uniform  ("offsetY", (float)ny);
  pass->pProgram->uniform  ("mOrtho", pRoidRage->ortho);
  pass->pProgram->uniform  ("mModel", pass->modelMatrix);
  pass->pProgram->uniform  ("vColor", pass->color);
  //pass->pProgram->uniform  ("depth",  2.0f);

  pass->pProgram->uniform  ("uTexture",  *pass->pTex[0]);
  pass->pProgram->attribute("vPosition", *pass->pVbo[0], 2, 4*sizeof(float), 0);
  pass->pProgram->attribute("vTexture",  *pass->pVbo[0], 2, 4*sizeof(float), 2*sizeof(float));

  pass->pProgram->execute  (0, 4);
}

//------------------------------------------------------------------------------

void renderFrame(RenderPass* pass, float width, float height, float scale) 
{
  pRoidRage->ortho = glm::ortho(0.0f, 
                                (float)width, 
                                (float)height, 
                                0.0f, 
                                -1.0f, 1.0f);


  pass->modelMatrix = glm::scale(
                        glm::translate(
                          glm::mat4(),
                          glm::vec3(width/2.0f, height/2.0f, 0.0f)
                        ),
                        glm::vec3(width/2.0f, -height/2.0f, 1.0f)
                      );

  pass->pProgram->use();

  pass->pProgram->uniform  ("fade",   scale);
  pass->pProgram->uniform  ("mOrtho", pRoidRage->ortho);
  pass->pProgram->uniform  ("mModel", pass->modelMatrix);
  pass->pProgram->uniform  ("vColor", pass->color);

  pass->pProgram->uniform  ("uGlowFrame",*pass->pTex[0]);
  pass->pProgram->attribute("vPosition", *pass->pVbo[0], 2, 4*sizeof(float), 0);
  pass->pProgram->attribute("vTexture",  *pass->pVbo[0], 2, 4*sizeof(float), 2*sizeof(float));

  pass->pProgram->execute  (0, 4);
}

//------------------------------------------------------------------------------

};
