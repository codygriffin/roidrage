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
  Target(const std::string& e) : position(game_.entity(e).get<Position>()->pos) {}
};

struct Orbit : public Component<Orbit> {
  float radius;
  glm::vec2   startingPoint;
  bool        transfer; 
  std::string entity;
  std::string body;
  Orbit(float r, const glm::vec2& s, const std::string& e, const std::string& b) 
    : radius(r), startingPoint(s), entity(e), body(b), transfer(true) {}
};

static float zoom = 1.0f;
struct Projection : public Component<Projection> {
  glm::mat4 matrix;
  Projection(float width, float height, float zoom = 1.0f) 
    : matrix (glm::ortho((-width/2.0f / zoom), 
                         (width/2.0f / zoom), 
                         (height/2.0f / zoom), 
                         (-height/2.0f / zoom), 
                          -1.0f, 1.0f)) {}
};

struct Pickable : boson::Component<Pickable> {
  std::string entity;
  Pickable(const std::string& name) : entity(name) {}
};

struct HillSphere  : public boson::Component<HillSphere> {
  float mag;
  std::string entity;
  HillSphere(float r, std::string& name) : mag(r), entity(name) {}
};


// Helpers for orbital mechanics stuff
namespace orbital {
  float speedAtRadius(float r, float m) {
    return sqrtf(Mass::unit * m/r);
  }
};

namespace events {
  struct Collision {};
  struct LeftClick {};
  struct RightClick {};
}

//------------------------------------------------------------------------------

Entity& createIndicator(Entity& e);
struct Selection {
  static void clear(){
    for (auto kv : selected) {
      game_.remove(kv.second->name());
    }
    selected.clear();
  }

  static void select(Entity& e) {
    auto item = selected.find(&e);
    if (item == selected.end()) {
      selected.emplace(std::make_pair(&e, &createIndicator(e)));
    }
  }
  static void unselect(Entity& e) {
    auto item = selected.find(&e);
    if (item != selected.end()) {
      game_.remove(item->second->name());
      selected.erase(item);
    }
  }

  static bool isSelected(Entity& e) {
    auto item = selected.find(&e);
    if (item != selected.end()) {
      return true;
    }
    return false;
  }

  static std::map<Entity*, Entity*> selected;
};
std::map<Entity*, Entity*> Selection::selected;

struct Picker {
  glm::vec2 position;
  bool      clicked;
  Entity*   picked;

  Picker(const glm::vec2& p) 
    : position (p)
    , clicked (false)
    , picked(0) {
  }

  Entity* query() {
    auto p = picked;
    picked = 0;
    return p;
  }

  void 
  testPickable(Pickable* s, Position* p, Radius* r) {
    static const float paddingFactor = 2.2f;  
    // TODO abort system exec()
    if (clicked == true) return;

    auto displacement = p->pos - position;

    auto& entity = game_.entity(s->entity);

    if (paddingFactor*r->mag > glm::length(displacement)) { 
      clicked = true;
      picked = &entity;
    } 
  }

  void 
  testHillSphere(Position* p, HillSphere* r/*, EntityRef* e*/) {
    // TODO abort system exec()
    if (clicked == true) return;

    auto displacement = p->pos - position;

    auto& entity = game_.entity(r->entity);

    if (r->mag > glm::length(displacement)) { 
      clicked = true;
      picked = &entity;
    } 
  }
};

struct Acceleration {
  void reset(Position* pPos) {
    pPos->acc  = glm::vec2(0.0f);
    pPos->aacc = 0.0f;
  }

  //------------------------------------------------------------------------------

  void update(Position* p1, Mass* m1) {
    current_ = std::make_tuple(p1, m1);
    game_.exec(*this, &Acceleration::update2);
  }

  //------------------------------------------------------------------------------

  std::tuple<Position*, Mass*> current_;
  void update2(Position* p2, Mass* m2) {
    Position* p1; 
    Mass*     m1; 
    std::tie(p1, m1) = current_;
    if (p1 == p2) return;

    glm::vec2 diff = p2->pos - p1->pos;
    float r = glm::length(diff);

    // Clamp to sane values
    if (r <  10.0f)          return;
    if (r >  1500.0f)        return;
    if (m1->mag < 0.00001f)  return;

    const float mr2 = m1->mag*r*r;
    glm::normalize(diff);

    glm::vec2 acc  = diff *   Mass::unit*m1->mag*m2->mag / mr2;

    p1->acc += acc;
    if (p1->acc.x != p1->acc.x && p1->acc.y != p1->acc.y) {
      Log::error("Acceleration is NaN after update");
      p1->acc = glm::vec2(0.0f, 0.0f);
    }
  }
};

struct CollisionDetector {
  void update(Handler<events::Collision>* c1, Position* p1, Radius* r1) {
    current_ = std::make_tuple(c1, p1, r1);
    game_.exec(*this, &CollisionDetector::update2);
  }

  //------------------------------------------------------------------------------

  void update2(Handler<events::Collision>* c2, Position* p2, Radius* r2) {
    static const float paddingFactor = 0.8f;  

    Handler<events::Collision>* c1; 
    Position* p1; 
    Radius*   r1; 
    std::tie(c1, p1, r1) = current_;
    if (p1 == p2) return;

    // Boundary is the minimum allowable distance 
    // between colliding entities
    auto boundary = r1->mag + r2->mag;

    // Displacement is the vector pointing from this entity to the 
    // other entity, with a length corresponding to the distance
    auto displacement = p1->pos - p2->pos;

    if (boundary * paddingFactor > glm::length(displacement)) { 
      // Really, we only care about overlap in the collision
      displacement = glm::normalize(displacement) * 
                    (boundary - glm::length(displacement));
      //Collision collision(displacement, boundary);
      c2->handler(events::Collision());
      //b2->onCollideWith(b1, &collision);
    }
  }

  std::tuple<Handler<events::Collision>*, Position*, Radius*> current_;
};

//------------------------------------------------------------------------------

void 
updateTime(Time* time) {
  float current = glfwGetTime();
  time->vel = current - time->pos;
  time->pos = current; 
}

//------------------------------------------------------------------------------

void updatePosition(Time* time, Position* pos) {
  //TODO better integration (RK4?)
  pos->vel     += pos->acc     * float(time->vel);
  pos->avel    += pos->aacc    * float(time->vel);

  pos->pos     += pos->vel     * float(time->vel);
  pos->apos    += pos->avel    * float(time->vel);

  // Wrap 
  if (pos->apos >  360.0f) {
    pos->apos = 0.0f;
  }
  if (pos->apos < -360.0f) {
    pos->apos = 0.0f;
  }

  // Clamp 
  pos->avel = std::min(pos->avel,  3.0f);
  pos->avel = std::max(pos->avel, -3.0f);
  pos->aacc = std::min(pos->aacc,  0.004f);
  pos->aacc = std::max(pos->aacc, -0.004f);
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
orbit(Position* p, Mass* m, Orbit* o) {
  auto body         = game_.entity(o->body);
  auto displacement = p->pos - body.get<Position>()->pos;
  auto dir = glm::normalize(displacement);

  o->radius = std::max(o->radius, body.get<Radius>()->mag*1.1f);

  auto mass = m->mag + body.get<Mass>()->mag;
  auto targetSpeed = orbital::speedAtRadius(o->radius, mass);
  auto currentSpeed = glm::length(p->vel);
  auto targetVelocity = targetSpeed * glm::vec2(-displacement.y, displacement.x) + body.get<Position>()->vel;
  auto posError = o->radius * dir - displacement;
  auto velError = targetVelocity  - p->vel;

  if (!o->transfer) {
    p->pos += posError;
    p->acc = glm::vec2();
    p->vel = targetVelocity;
    dir    = glm::normalize(p->vel);
  } else {

    p->acc += 0.6f  * velError; 
    p->vel += 0.04f * posError;

    dir    = glm::normalize(p->acc);
    Log::debug("transfer %", glm::length(posError));
    if (glm::length(posError) < (0.01f * o->radius)) {
      Log::debug("ending transfer");
      o->transfer = false;
    }
  }

  p->apos = atan2(dir.y, dir.x) * 360.0f/6.28f + 90.0f;

  if (glm::length(p->vel) > 1e13) {
    throw std::runtime_error("runaway!");
  }
}

//------------------------------------------------------------------------------

void renderSolid(Transform* x, Color* c, GlProgram* p, GlVbo* v) {
  auto matProjection = game_.entity("camera").get<Projection>()->matrix;
  auto matView       = game_.entity("camera").get<Transform>()->transform;
  p->program.use();
  p->program.uniform("mOrtho",      matProjection);
  p->program.uniform("mModel",      matView * x->transform);
  p->program.uniform("vColor",      c->color);
  p->program.attribute("vPosition", v->vbo, 2, 4*sizeof(float), 0);
  p->program.attribute("vTexture",  v->vbo, 2, 4*sizeof(float), 2*sizeof(float));
  p->program.execute  (0, 4);
}

void renderTextured(Transform* x, GlTexture* t, GlProgram* p, GlVbo* v) {
  auto matProjection = game_.entity("camera").get<Projection>()->matrix;
  auto matView       = game_.entity("camera").get<Transform>()->transform;
  p->program.use();
  p->program.uniform("mOrtho",      matProjection);
  p->program.uniform("mModel",      matView * x->transform);
  p->program.uniform("uTexture",    t->texture);
  p->program.attribute("vPosition", v->vbo, 2, 4*sizeof(float), 0);
  p->program.attribute("vTexture",  v->vbo, 2, 4*sizeof(float), 2*sizeof(float));
  p->program.execute  (0, 4);
}

//------------------------------------------------------------------------------

void removeFromGame(Entity& e) {
  gameQueue_.enqueue( [&] () { game_.remove(e.name()); });
}

Entity& createIndicator(Entity& entity) {
  auto& indicator = game_.entity();
  Log::debug("Created indicator[%]", indicator.name());
  indicator.add<Position>();
  indicator.add<Radius>(entity.get<Radius>()->mag);
  indicator.add<Transform>(entity.get<Transform>());
  indicator.add<Color>(0.8, 0.9, 0.8, 0.8f);
  indicator.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                           FragmentShader("assets/gpu/selected.fp"));
  // TODO do we want Geometry, Textures and Programs to be components?
  static const GLfloat pQuad[] = {1.0,  1.0, 1.0, 1.0,  1.0, -1.0, 1.0, 0.0,
                                 -1.0, -1.0, 0.0, 0.0, -1.0,  1.0, 0.0, 1.0};
  indicator.add<GlVbo>(16*sizeof(GLfloat), pQuad);
  return indicator;
}

//------------------------------------------------------------------------------

Entity& createShip(float x, float y, float r) {
  auto& ship = game_.entity();
  Log::debug("Created ship[%]", ship.name());

  ship.add<Time>();
  ship.add<Position>();
  ship.get<Position>()->pos = glm::vec2(x, y);
  ship.add<Radius>(r);
  ship.add<Mass>(r*r*r);
  ship.add<Pickable>(ship.name());
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
  ship.add<GlVbo>(sizeof(quad), quad);
  ship.add<GlTexture>(512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/ship01.png",    w_, h_).get());
  return ship;
}

Entity& createBoid(float x, float y, float r) {
  auto& ship = game_.entity();
  Log::debug("Created ship[%]", ship.name());

  ship.add<Time>();
  ship.add<Position>();
  ship.get<Position>()->pos = glm::vec2(x, y);
  ship.add<Radius>(r);
  ship.add<Mass>(r*r*r);
  ship.add<Pickable>(ship.name());
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
  ship.add<GlVbo>(sizeof(quad), quad);
  ship.add<GlTexture>(512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/boid01.png",    w_, h_).get());
  return ship;
}

Entity& createRoid(float x, float y, float r) {
  auto& roid = game_.entity();
  Log::debug("Created asteroid[%]: %", roid.name(), &roid);

  roid.add<Time>();
  roid.add<Position>();
  roid.get<Position>()->pos = glm::vec2(x, y);
  roid.add<Radius>(r);
  roid.add<HillSphere>(5.0f * r, roid.name());
  roid.add<Mass>(r*r*r);
  roid.add<Transform>();
  roid.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                      FragmentShader("assets/gpu/texture.fp"));

  roid.add<Handler<events::Collision>>([&](events::Collision) {
    //removeFromGame(roid);
  });


  // TODO do we want Geometry, Textures and Programs to be components?
  //
  const GLfloat quad[] = {1.0,  1.0, 1.0, 1.0,
                          1.0, -1.0, 1.0, 0.0,
                         -1.0, -1.0, 0.0, 0.0,
                         -1.0,  1.0, 0.0, 1.0};
  unsigned w_, h_;
  roid.add<GlVbo>(sizeof(quad), quad);
  roid.add<GlTexture>(512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/astroid01.png",    w_, h_).get());
  return roid;
}

Entity& createGasPlanet(float x, float y, float r) {
  auto& roid = game_.entity();
  Log::debug("Created gas[%]: %", roid.name(), &roid);

  roid.add<Time>();
  roid.add<Position>();
  roid.get<Position>()->pos = glm::vec2(x, y);
  roid.add<Radius>(r);
  roid.add<HillSphere>(2.0f * r, roid.name());
  roid.add<Mass>(r*r*r);
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
  roid.add<GlVbo>(sizeof(quad), quad);
  roid.add<GlTexture>(512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/gas01.png",    w_, h_).get());
  return roid;
}


//------------------------------------------------------------------------------

RoidRageGameTesting::RoidRageGameTesting(RoidRage* pMachine) 
  : RoidRage::State(pMachine) 
{ 
  game_.registerIndex(updateTime);

  game_.registerIndex(&Acceleration::reset);
  game_.registerIndex(orbit);
  game_.registerIndex(&Acceleration::update);
  game_.registerIndex(updatePosition);

  game_.registerIndex(&Transformations::transform1);
  game_.registerIndex(&Transformations::transform2);
  game_.registerIndex(renderSolid);
  game_.registerIndex(renderTextured);

  game_.registerIndex(&Picker::testPickable);
  game_.registerIndex(&Picker::testHillSphere);
  game_.registerIndex(&CollisionDetector::update);

  game_.registerEvent<events::Collision>();

  auto& cam = game_.entity("camera");
  cam.add<Projection>(Display::getWidth(), Display::getHeight(), 1.0f);
  cam.add<Time>();
  cam.add<Position>();
  cam.get<Position>()->pos = glm::vec2();
  cam.add<Transform>();

  auto& gas = createGasPlanet(0.0f, 0.0f, 500.0f);

  for (unsigned i = 0; i < 5; i++) {
    auto& ship = createShip(100.0f + rand() % 100, rand() % 100, 20.0f);
    ship.add<Orbit>(600.0f + i*30.0f, ship.get<Position>()->pos, ship.name(), gas.name());
  }

  for (unsigned i = 0; i < 5; i++) {
    auto& ship = createBoid(100.0f + i*10.0f, i*10.0f, 20.0f);
    ship.add<Orbit>(600.0f + rand() % 100, ship.get<Position>()->pos, ship.name(), gas.name());
  }

  for (unsigned i = 0; i < 5; i++) {
    auto& roid = createRoid(100.0f - (rand() % 200), 100.0f - (rand() % 200), 100.0f + rand() % 100);
    roid.add<Orbit>(1000.0f + rand() % 1000, roid.get<Position>()->pos, roid.name(), gas.name());
  }

  timerReactor_.setPeriodic(std::chrono::seconds(5), [&] () {
    gameQueue_.enqueue( [&] () {
    });
  });
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(Tick tick) {
  static Acceleration       acceleration;
  static Transformations    transforms;
  static CollisionDetector collisions;

  game_.exec(acceleration, &Acceleration::reset); 

  // Begin FSM stuff
  game_.exec(orbit); 

  //game_.exec(acceleration, &Acceleration::update); 
  game_.exec(updateTime); 
  game_.exec(updatePosition); 

  game_.exec(transforms, &Transformations::transform1); 
  transforms.exec();

  game_.exec(transforms, &Transformations::transform2); 

  game_.exec(collisions, &CollisionDetector::update); 

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
  float scrollSpeed = 300.0f / zoom;

  auto& cam = game_.entity("camera");

  switch (key.key) {
    case GLFW_KEY_W:
      if (key.action == GLFW_PRESS) {
        cam.get<Position>()->vel += glm::vec2(0.0f, scrollSpeed);
      }
      if (key.action == GLFW_RELEASE) {
        cam.get<Position>()->vel -= glm::vec2(0.0f, scrollSpeed);
      }
    break;
    case GLFW_KEY_A:
      if (key.action == GLFW_PRESS) {
        cam.get<Position>()->vel += glm::vec2(scrollSpeed, 0.0f);
      }
      if (key.action == GLFW_RELEASE) {
        cam.get<Position>()->vel -= glm::vec2(scrollSpeed, 0.0f);
      }
    break;
    case GLFW_KEY_S:
      if (key.action == GLFW_PRESS) {
        cam.get<Position>()->vel += glm::vec2(0.0f, -scrollSpeed);
      }
      if (key.action == GLFW_RELEASE) {
        cam.get<Position>()->vel -= glm::vec2(0.0f, -scrollSpeed);
      }
    break;
    case GLFW_KEY_D:
      if (key.action == GLFW_PRESS) {
        cam.get<Position>()->vel += glm::vec2(-scrollSpeed, 0.0f);
      }
      if (key.action == GLFW_RELEASE) {
        cam.get<Position>()->vel -= glm::vec2(-scrollSpeed, 0.0f);
      }
    break;
  
  }
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(GlfwMouseMove mouse) {
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(GlfwMouseScroll mouse) {
  static float zoomSensitivity = 0.1f;
  zoom += mouse.y * zoomSensitivity;
  zoom = std::min(zoom,  5.0f);
  zoom = std::max(zoom,  0.1f);
  auto& cam = game_.entity("camera");
  cam.replace<Projection>(Display::getWidth(), Display::getHeight(), zoom); 
  game_.entity("camera").get<Position>()->vel = glm::vec2(0.0f, 0.0f);
}

//------------------------------------------------------------------------------
// this is a hot mess
void 
RoidRageGameTesting::onEvent(GlfwMouseButton mouse) {
  auto& cam = game_.entity("camera");
  auto position = glm::vec2(mouse.x - Display::getWidth()/2.0f, 
                            mouse.y - Display::getHeight()/2.0f);
  position /= zoom;
  position -= cam.get<Position>()->pos;
  Picker picker(position);
  game_.exec(picker, &Picker::testPickable); 

  if      (mouse.button == GLFW_MOUSE_BUTTON_LEFT && mouse.action == GLFW_PRESS) {
    if (!mouse.mods & GLFW_MOD_SHIFT) {
      Selection::clear(); 
    }
    if (auto e = picker.query()) {
      Selection::select(*e);
    }   
  }

  else if (mouse.button == GLFW_MOUSE_BUTTON_RIGHT && mouse.action == GLFW_PRESS) {
    game_.exec(picker, &Picker::testHillSphere); 
    for (auto kv : Selection::selected) {
      auto s = kv.first;
      auto h = picker.query();
      if (h && h != s) {
        float r = glm::length(position - h->get<Position>()->pos);
        Log::debug("In hill sphere of entity[%]: r=%", h->name(), r);
        s->addOrReplace<Orbit>(r, s->get<Position>()->pos, s->name(), h->name());
      }
    }
  }
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(AndroidBack back) {
  getMachine()->transition<RoidRageMenu>(true);
}

//------------------------------------------------------------------------------
