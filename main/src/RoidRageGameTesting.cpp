//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "RoidRageGameTesting.h"
#include "RoidRageMenu.h"
#include "RoidRageGameOver.h"

#include "Shader.h"
#include "Program.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "VertexBufferObject.h"
#include "OrthoCamera.h"

#include "AssetManager.h"
#include "RenderState.h"
#include "Display.h"

#include "Renderable.h"

#include "Entity.h"
//#include "Systems.h"

#include "Log.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace roidrage;
using namespace corvid;

//------------------------------------------------------------------------------


static  System               game_;
static  corvid::WorkQueue    gameQueue_;

struct Time : Component<Time> {
  float pos;
  float vel;
  Time() : pos(0), vel(0) {}
};

struct Target : public Component<Target> {
  glm::vec2 position;
  Target(const glm::vec2& p) : position(p) {}
};

//------------------------------------------------------------------------------

void 
updateTime(Time* time) {
  float current = glfwGetTime();
  time->vel = current - time->pos;
  time->pos = current; 
  //Log::debug("Tick ∂=%", time->vel);
}

//------------------------------------------------------------------------------

void updatePosition(Time* time, Position* pos) {
  // Clamp that shit down - what about lightspeed/relativity?
  //if (glm::length(pos->vel) > pos->maxVel) {
  //  pos->vel = glm::normalize(pos->vel) * pos->maxVel;
  //}

  //TODO better integration (RK4?)
  pos->vel     += pos->acc     * float(time->vel);
  pos->avel    += pos->aacc    * float(time->vel);

  pos->pos     += pos->vel     * float(time->vel);
  pos->apos    += pos->avel    * float(time->vel);

  if (glm::length(pos->vel) > 0.0f) {
    glm::vec2 dir = glm::normalize(pos->vel);
    pos->apos = atan2(dir.y, dir.x) * 360.0f/6.28f + 90.0f;
  }

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
  pos->distance += glm::length(pos->vel * float(time->vel));
}

//------------------------------------------------------------------------------

void 
transform1(Transform* x, Position* p) {
  x->transform = glm::mat4();

  // Apply parent transformations
  // TODO order of this index matters now (parent/child dependency between transforms)
  if (x->parent) { 
    x->transform = x->parent->transform;   
  } 

  // Set our position, rotation and scale according
  // to the position, radius and angular position.
  x->transform = glm::translate(
    x->transform, 
    glm::vec3(p->pos.x, p->pos.y, 0.0f)
  );

  x->transform = glm::rotate(
    x->transform, 
    glm::radians(p->apos),
    glm::vec3(0.0f,0.0f,1.0f)
  );
}

void 
transform2(Transform* x, Radius* r) {
  x->transform = glm::scale(
    x->transform, 
    glm::vec3(r->mag)
  );
}

//------------------------------------------------------------------------------

void
target(Position* p, Target* t) {
  p->vel = t->position - p->pos;
  if (glm::length(p->vel) > 1e06f) {
    throw std::runtime_error("runaway rocket!");
  }
}

//------------------------------------------------------------------------------

glm::vec2 Selectable::position;
void 
onClick(Selectable* s, Position* p, Radius* r) {
  // Displacement is the vector pointing from this entity to the 
  // other entity, with a length corresponding to the distance
  auto displacement = p->pos - Selectable::position;
  Log::debug("testing @(%,%) for position (%,%) radius %", 
             Selectable::position.x, Selectable::position.y,
             p->pos.x, p->pos.y,
             r->mag);

  auto diff = Selectable::position - p->pos;
  Log::debug("∂(%,%) m=%", 
             diff.x, diff.y, glm::length(diff));

  const float paddingFactor = 2.0f;  
  if (paddingFactor*r->mag > glm::length(displacement)) { 
    if (!s->selected) {
      s->selected = true;
      auto& indicator = game_.entity();
      s->indicator = indicator.name();
      Log::debug("Created indicator[%]", indicator.name());
      indicator.add<Position>();
      indicator.add<Radius>(90.0f);
      indicator.add<Transform>(game_.entity("ship").get<Transform>());
      indicator.add<Color>(0.8, 0.9, 0.8, 0.8f);
      indicator.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                               FragmentShader("assets/gpu/selected.fp"));
      // TODO do we want Geometry, Textures and Programs to be components?
      //
      const GLfloat pQuad[] = {1.0,  1.0, 1.0, 1.0,
                               1.0, -1.0, 1.0, 0.0,
                              -1.0, -1.0, 0.0, 0.0,
                              -1.0,  1.0, 0.0, 1.0};
      unsigned w_, h_;
      indicator.add<GlVbo>(16*sizeof(GLfloat), pQuad);
    } else {
      auto& indicator = game_.entity(s->indicator);
      Log::debug("Removing indicator[%]", indicator.name());
      game_.remove(s->indicator);
      //indicator.clear();
      s->indicator = "";
      s->selected = false;
    }
  } else {
    gameQueue_.enqueue( [&] () {
      auto& ship = game_.entity("ship");
      if (ship.get<Selectable>()->selected) {
        Log::debug("old target @(%,%)", 
                    ship.get<Target>()->position.x, ship.get<Target>()->position.y);
        ship.replace<Target>(Selectable::position);
        Log::debug("new target @(%,%)", 
                    ship.get<Target>()->position.x, ship.get<Target>()->position.y);
      }
    });
  }
}

//------------------------------------------------------------------------------

void renderSolid(Transform* x, Color* c, GlProgram* p, GlVbo* v) {
//void render(Quad* pRenderable) {
//  pRenderable->render(RenderState::pCam_.get());
//}
  p->program.use();
  p->program.uniform("mOrtho", RenderState::pCam_->getOrthoMatrix());
  p->program.uniform("mModel", x->transform);
  p->program.uniform("vColor", c->color);

  p->program.attribute("vPosition", v->vbo, 2, 4*sizeof(float), 0);
  p->program.attribute("vTexture",  v->vbo, 2, 4*sizeof(float), 2*sizeof(float));

  p->program.execute  (0, 4);
}

void renderTextured(Transform* x, GlTexture* t, GlProgram* p, GlVbo* v) {
  p->program.use();
  p->program.uniform("mOrtho", RenderState::pCam_->getOrthoMatrix());
  p->program.uniform("mModel", x->transform);
  p->program.uniform("uTexture",  t->texture);

  p->program.attribute("vPosition", v->vbo, 2, 4*sizeof(float), 0);
  p->program.attribute("vTexture",  v->vbo, 2, 4*sizeof(float), 2*sizeof(float));

  p->program.execute  (0, 4);
}

//------------------------------------------------------------------------------

RoidRageGameTesting::RoidRageGameTesting(RoidRage* pMachine) 
  : RoidRage::State(pMachine) 
{ 
  game_.registerIndex(updateTime);
  game_.registerIndex(updatePosition);
  game_.registerIndex(target);
  game_.registerIndex(transform1);
  game_.registerIndex(transform2);
  game_.registerIndex(renderSolid);
  game_.registerIndex(renderTextured);
  game_.registerIndex(onClick);

  auto& ship = game_.entity("ship");
  Log::debug("Created ship[%]", ship.name());

  ship.add<Time>();
  ship.get<Time>()->pos = 0.0f;
  ship.get<Time>()->vel = 0.016f;
  ship.add<Position>();
  ship.add<Target>(glm::vec2());
  ship.add<Radius>(90.0f);
  ship.add<Selectable>(false);
  ship.add<Transform>();
  ship.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                      FragmentShader("assets/gpu/texture.fp"));
  // TODO do we want Geometry, Textures and Programs to be components?
  //
  const GLfloat pQuad[] = {1.0,  1.0, 1.0, 1.0,
                           1.0, -1.0, 1.0, 0.0,
                          -1.0, -1.0, 0.0, 0.0,
                          -1.0,  1.0, 0.0, 1.0};
  unsigned w_, h_;
  ship.add<GlVbo>(16*sizeof(GLfloat), pQuad);
  ship.add<GlTexture>(512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/ship01.png",    w_, h_).get());

  timerReactor_.setPeriodic(std::chrono::seconds(5), [&] () {
    gameQueue_.enqueue( [&] () {
      Log::debug("<Every 5 Seconds>");
    });
  });
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(Tick tick) {
  game_.visit(updateTime); 
  game_.visit(updatePosition); 
  game_.visit(target); 
  game_.visit(transform1); 
  game_.visit(transform2); 

  pRoidRage->ortho = RenderState::pCam_->getOrthoMatrix(); 
  glClearColor(0.07f, 0.07f, 0.13f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  game_.visit(renderSolid); 
  game_.visit(renderTextured); 

  // execute actions 'scheduled' for this tick
  while (gameQueue_.work());
}

//------------------------------------------------------------------------------
// TODO definitely a better way for this - perhaps a map of keys and lambdas?
void 
RoidRageGameTesting::onEvent(GlfwKey key) {
  switch (key.key) {
  }
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(GlfwMouseMove mouse) {
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(GlfwMouseButton mouse) {
  if (mouse.button == GLFW_MOUSE_BUTTON_LEFT && mouse.action == GLFW_PRESS) {
    auto screen = glm::vec2(mouse.x, mouse.y);
    auto world = RenderState::pCam_->toWorld(screen);
    Selectable::position = world;
    game_.visit(onClick); 
  }
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(AndroidBack back) {
  getMachine()->transition<RoidRageMenu>(true);
}

//------------------------------------------------------------------------------
