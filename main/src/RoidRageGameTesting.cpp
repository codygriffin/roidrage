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

#include "AssetManager.h"
#include "Display.h"

#include "Entity.h"
#include "System.h"
//#include "Systems.h"

#include "Log.h"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <map>

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace roidrage;
using namespace corvid;
using namespace boson;

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

struct Layer : public Component<Layer> {
  int layer;
  Layer(int l = 0) : layer(l) {}
  bool operator<(const Layer& rhs) {
    return layer < rhs.layer;
  }
};

struct Projection : public Component<Projection> {
  glm::mat4 matrix;
  Projection(float width, float height) 
    : matrix (glm::ortho((-width/2.0f), 
                         (width/2.0f), 
                         (height/2.0f), 
                         (-height/2.0f), 
                          -1.0f, 1.0f)) {}
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

struct Transformations {
  void 
  transform1(Transform* x, Position* p) {
    transforms_.emplace(std::make_pair(x->depth(), [=]() {
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
    }));
  }

  void 
  transform2(Transform* x, Radius* r) {
    x->transform = glm::scale(
      x->transform, 
      glm::vec3(r->mag)
    );
  }
  
  void
  exec() {
    for (auto kv : transforms_) {
      kv.second();
    }
    transforms_.clear();
  }

  std::multimap<unsigned, std::function<void (void)>> transforms_;
};

//------------------------------------------------------------------------------

void
target(Position* p, Target* t) {
  static const float cruisingSpeed = 100.0f;
  static const float engineThrust  = 1.0f;
  
 // p->vel = t->position - p->pos;
  auto distance     = t->position - p->pos;
  auto velocityDiff = -p->vel; 
  p->acc = distance + velocityDiff;

  if (glm::length(p->acc) > 0.0f) {
    glm::vec2 dir = glm::normalize(p->vel);
    p->apos = atan2(dir.y, dir.x) * 360.0f/6.28f + 90.0f;
  }
  

  if (glm::length(p->vel) > 1e06f) {
    throw std::runtime_error("runaway rocket!");
  }
}

//------------------------------------------------------------------------------

Entity& createIndicator(Entity& ship) {
  auto& indicator = game_.entity();
  Log::debug("Created indicator[%]", indicator.name());
  indicator.add<Position>();
  indicator.add<Layer>(3);
  indicator.add<Radius>(ship.get<Radius>()->mag);
  indicator.add<Transform>(ship.get<Transform>());
  indicator.add<Color>(0.8, 0.9, 0.8, 0.8f);
  indicator.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                           FragmentShader("assets/gpu/selected.fp"));
  // TODO do we want Geometry, Textures and Programs to be components?
  static const GLfloat pQuad[] = {1.0,  1.0, 1.0, 1.0,  1.0, -1.0, 1.0, 0.0,
                                 -1.0, -1.0, 0.0, 0.0, -1.0,  1.0, 0.0, 1.0};
  indicator.add<GlVbo>(16*sizeof(GLfloat), pQuad);
  return indicator;
}

// some nasty global state
glm::vec2 Selectable::position;
bool      Selectable::clicked;

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
  auto& ship = game_.entity(s->entity);

  const float paddingFactor = 1.2f;  
  if (paddingFactor*r->mag > glm::length(displacement)) { 
    if (Selectable::clicked == true) return;
    Selectable::clicked = true;

    if (!s->selected) {
      s->selected = true;
      auto& indicator = createIndicator(ship);
      s->indicator = indicator.name();
    } else {
      auto& indicator = game_.entity(s->indicator);
      Log::debug("Removing indicator[%]", indicator.name());
      game_.remove(s->indicator);
      //indicator.clear();
      s->indicator = "";
      s->selected = false;
    }
  } else {
    if (ship.get<Selectable>()->selected) {
      gameQueue_.enqueue( [&] () {
          ship.replace<Target>(Selectable::position);
      });
    }
  }
}

//------------------------------------------------------------------------------

void renderSolid(Layer* l, Transform* x, Color* c, GlProgram* p, GlVbo* v) {
  auto matProjection = game_.entity("camera").get<Projection>()->matrix;
  auto matView       = game_.entity("camera").get<Transform>()->transform;
  p->program.use();
  p->program.uniform("mOrtho",      matProjection);
  p->program.uniform("mModel",      x->transform);
  p->program.uniform("vColor",      c->color);
  p->program.attribute("vPosition", v->vbo, 2, 4*sizeof(float), 0);
  p->program.attribute("vTexture",  v->vbo, 2, 4*sizeof(float), 2*sizeof(float));
  p->program.execute  (0, 4);
}

void renderTextured(Layer* l, Transform* x, GlTexture* t, GlProgram* p, GlVbo* v) {
  p->program.use();
  p->program.uniform("mOrtho",      game_.entity("camera").get<Projection>()->matrix);
  p->program.uniform("mModel",      x->transform);
  p->program.uniform("uTexture",    t->texture);
  p->program.attribute("vPosition", v->vbo, 2, 4*sizeof(float), 0);
  p->program.attribute("vTexture",  v->vbo, 2, 4*sizeof(float), 2*sizeof(float));
  p->program.execute  (0, 4);
}

//------------------------------------------------------------------------------

Entity& createShip(float x, float y, float r) {
  auto& ship = game_.entity();
  Log::debug("Created ship[%]", ship.name());

  ship.add<Layer>(2);
  ship.add<Time>();
  ship.add<Position>();
  ship.get<Position>()->pos = glm::vec2(x, y);
  ship.add<Target>(glm::vec2(x, y));
  ship.add<Radius>(r);
  ship.add<Selectable>(false, ship.name());
  ship.add<Transform>();
  ship.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                      FragmentShader("assets/gpu/texture.fp"));
  // TODO do we want Geometry, Textures and Programs to be components?
  //
  const GLfloat quad[] = {1.0,  1.0, 1.0, 1.0,
                          1.0, -1.0, 1.0, 0.0,
                         -1.0, -1.0, 0.0, 0.0,
                         -1.0,  1.0, 0.0, 1.0};
  unsigned w_, h_;
  //ship.add<GlVbo>(16*sizeof(GLfloat), pQuad);
  ship.add<GlVbo>(sizeof(quad), quad);
  ship.add<GlTexture>(512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/ship01.png",    w_, h_).get());
  return ship;
}

Entity& createRoid(float x, float y, float r) {
  auto& roid = game_.entity();
  Log::debug("Created asteroid[%]", roid.name());

  roid.add<Layer>(1);
  roid.add<Time>();
  roid.add<Position>();
  roid.get<Position>()->pos = glm::vec2(x, y);
  roid.add<Target>(glm::vec2(x, y));
  roid.add<Radius>(r);
  roid.add<Selectable>(false, roid.name());
  roid.add<Transform>();
  roid.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                      FragmentShader("assets/gpu/texture.fp"));
  // TODO do we want Geometry, Textures and Programs to be components?
  //
  const GLfloat quad[] = {1.0,  1.0, 1.0, 1.0,
                          1.0, -1.0, 1.0, 0.0,
                         -1.0, -1.0, 0.0, 0.0,
                         -1.0,  1.0, 0.0, 1.0};
  unsigned w_, h_;
  //roid.add<GlVbo>(16*sizeof(GLfloat), pQuad);
  roid.add<GlVbo>(sizeof(quad), quad);
  roid.add<GlTexture>(512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/astroid01.png",    w_, h_).get());
  return roid;
}

void
testOrderTransform(Transform* x) {
  Log::debug("transform depth: %", x->depth());
}

//------------------------------------------------------------------------------

RoidRageGameTesting::RoidRageGameTesting(RoidRage* pMachine) 
  : RoidRage::State(pMachine) 
{ 
  game_.registerIndex(updateTime);
  game_.registerIndex(updatePosition);
  game_.registerIndex(target);
  game_.registerIndex(&Transformations::transform1);
  game_.registerIndex(&Transformations::transform2);
  game_.registerIndex(renderSolid);
  game_.registerIndex(renderTextured);
  game_.registerIndex(onClick);

  auto& cam = game_.entity("camera");
  cam.add<Projection>(Display::getWidth(), Display::getHeight());
  cam.add<Time>();
  cam.add<Position>();
  cam.get<Position>()->pos = glm::vec2();
  cam.add<Transform>();

  int maxRadius = 200;
  for (unsigned i = 0; i < 2; i++) {
    float roidX = (rand() % (2*maxRadius)) - maxRadius;
    float roidY = (rand() % (2*maxRadius)) - maxRadius;
    createShip(roidX, roidY, 20.0f);
  }

  for (unsigned i = 0; i < 3; i++) {
    float roidX = (rand() % (2*maxRadius)) - maxRadius;
    float roidY = (rand() % (2*maxRadius)) - maxRadius;
    createRoid(roidX, roidY, 40.0f + (rand() % 200));
  }

  timerReactor_.setPeriodic(std::chrono::seconds(5), [&] () {
    gameQueue_.enqueue( [&] () {
      Log::debug("<Every 5 Seconds>");
    });
  });
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(Tick tick) {
  Transformations transforms;

  game_.exec(updateTime); 
  game_.exec(updatePosition); 
  game_.exec(target); 

  game_.exec(transforms, &Transformations::transform1); 
  transforms.exec();

  game_.exec(transforms, &Transformations::transform2); 

  glClearColor(0.07f, 0.07f, 0.13f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  game_.exec(renderTextured); 
  game_.exec(renderSolid); 

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
    auto matProjection = game_.entity("camera").get<Projection>()->matrix;
    auto matView       = game_.entity("camera").get<Transform>()->transform;
    auto modelView     = matProjection * matView;
    auto viewPort      = glm::vec4(0.0f, 0.0f, Display::getWidth(), Display::getHeight());

    auto screen = glm::vec3(mouse.x, mouse.y, 0.0f);
    auto world  = glm::unProject(screen,
                                 modelView,
                                 matProjection,
                                 viewPort);
    //Selectable::position = glm::vec2(world.x, world.y);
    Selectable::position = glm::vec2(mouse.x - Display::getWidth()/2.0f, mouse.y - Display::getHeight()/2.0f);
    Selectable::clicked = false;
    // TODO events?
    game_.exec(onClick); 
  }
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(AndroidBack back) {
  getMachine()->transition<RoidRageMenu>(true);
}

//------------------------------------------------------------------------------
