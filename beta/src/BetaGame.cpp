//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "BetaGame.h"

#include "Shader.h"
#include "Program.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "VertexBufferObject.h"

#include "AssetManager.h"
#include "Display.h"

#include "Entity.h"
#include "System.h"
#include "BetaParams.h"

#include "Log.h"
#include "WorkQueue.h"

#include <iterator>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <map>

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace corvid;
using namespace boson;
using namespace beta;

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
  glm::vec2   periapsis;
  float       eccentricity;
  float       phase;

  std::string entity;
  std::string focus;
  bool        verbose;

  Orbit(const glm::vec2& p, float ecc, float phs, const std::string& e, const std::string& b, bool v = false) 
    : periapsis(p), eccentricity(ecc), phase(phs), entity(e), focus(b), verbose(v) {}
};

static float zoom = 0.1f;
struct Projection : public Component<Projection> {
  glm::mat4 matrix;
  Projection(float width, float height, float zoom = 0.1f) 
    : matrix (glm::ortho((-width/2.0f / zoom), 
                         (width/2.0f / zoom), 
                         (height/2.0f / zoom), 
                         (-height/2.0f / zoom), 
                          1.0f, -1.0f)) {}
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

namespace events {
  struct Collision {};
  struct LeftClick {};
  struct RightClick {};
}

struct String : public boson::Component<String> {
  String(const std::string& s, float r = 20.0f) : str(s), size(r) {}
  std::string str;
  float       size;
};

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
  
  static void toggle(Entity& e) {
    if (isSelected(e)) {
      unselect(e);
    } else {
      select(e);
    } 
  }

  static std::map<Entity*, Entity*> selected;
};
std::map<Entity*, Entity*> Selection::selected;

struct Picker {
  glm::vec2           start;
  glm::vec2           end;
  std::list<Entity*>  picked;
  std::list<Entity*>::iterator current;

  Picker() 
    : start ()
    , end ()
    , picked() 
    , current(picked.end()) {
  }

  std::list<Entity*>& query() {
    return picked;
  }

  void
  startBox(const glm::vec2& p) {
    start = p;
    end   = p;
  }

  void
  updateBox(const glm::vec2& p) {
    end = p;
  }

  Entity* cycle() {
    if (current != picked.end()) {
      auto e = *current;
      current++;
      return e;
    } else {
      current = picked.begin();
      return *current;
    }
  }

  void reset() {
    picked.clear();
  }

  bool isRange() {
    return start != end;
  }

  bool intersects(glm::vec2 cp, float rr, glm::vec2 rp1, glm::vec2 rp2)
  {
     auto x1 = std::min(rp1.x, rp2.x);
     auto y1 = std::min(rp1.y, rp2.y);
     auto x2 = std::max(rp1.x, rp2.x);
     auto y2 = std::max(rp1.y, rp2.y);
     float closestX = (cp.x < x1 ? x1 : (cp.x > x2 ? x2 : cp.x));
     float closestY = (cp.y < y1 ? y1 : (cp.y > y2 ? y2 : cp.y));
     float dx = closestX - cp.x;
     float dy = closestY - cp.y;

     return ( dx * dx + dy * dy ) <= rr * rr;
  }

  void 
  testPickable(Pickable* s, Position* p, Radius* r) {
    static const float paddingFactor = 3.0f;  

    auto& entity      = game_.entity(s->entity);

    if (isRange()) {
      if (intersects(p->pos, r->mag, start, end)) { 
        picked.push_back(&entity);
      } 
    } else {
      auto displacement = p->pos - start;
      if (paddingFactor*r->mag > glm::length(displacement)) { 
        picked.push_back(&entity);
      } 
    }
  }

  void 
  testHillSphere(Position* p, HillSphere* r) {
    // TODO abort system exec()
    auto displacement = p->pos - start;

    auto& entity = game_.entity(r->entity);

    Log::debug("testing if p: (%,%) is in %'s hill sphere c: (%,%) r: %", 
               start.x, start.y, entity.name(), p->pos.x, p->pos.y, r->mag);
    if (r->mag > glm::length(displacement)) { 
      Log::debug("yep");
      picked.push_back(&entity);
    } else {
      Log::debug("nope");
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
  if (std::isnan(pos->acc.x) || std::isnan(pos->acc.y)) {
    throw std::runtime_error("NaN acceleration!");
  }

  pos->vel     += pos->acc     * float(time->vel);

  if (std::isnan(pos->vel.x) || std::isnan(pos->vel.y)) {
    throw std::runtime_error("NaN velocity!");
  }

  pos->pos     += pos->vel     * float(time->vel);

  if (std::isnan(pos->pos.x) || std::isnan(pos->pos.y)) {
    throw std::runtime_error("NaN position!");
  }
}

void track(Track<Position>* t, Position* p) {
  p->pos     = t->value->pos + glm::vec2(-100.0f, -100.0f);
}


void updateOrientation(Time* time, Orientation* o) {
  o->avel    += o->aacc    * float(time->vel);
  o->apos    += o->avel    * float(time->vel);

  // Wrap 
  if (o->apos >  360.0f) {
    o->apos = 0.0f;
  }
  if (o->apos < -360.0f) {
    o->apos = 0.0f;
  }

  // Clamp 
  o->avel = std::min(o->avel,  3.0f);
  o->avel = std::max(o->avel, -3.0f);
  o->aacc = std::min(o->aacc,  0.004f);
  o->aacc = std::max(o->aacc, -0.004f);
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
    }));
  }

  void 
  transform2(Transform* x, Orientation* o) {
    transforms_.emplace(std::make_pair(x->depth(), [=]() {
      x->transform = glm::rotate(
        x->transform, 
        glm::radians(o->apos),
        glm::vec3(0.0f,0.0f,1.0f)
      );
    }));
  }

  void 
  transform3(Transform* x, Radius* r) {
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
orbit(Position* p, Mass* m, Orbit* o, Orientation *b) {
  auto focus = game_.entity(o->focus);
  auto d     = p->pos - focus.get<Position>()->pos;
  auto dir   = glm::normalize(d);
  auto r     = glm::length(d);
  auto e     = o->eccentricity;
  auto ra = glm::length(o->periapsis) * (1.0f + e)/(1.0f - e);
  auto rp    = o->periapsis + focus.get<Position>()->pos;


  float u     = Mass::unit*focus.get<Mass>()->mag;
  float y     = glm::dot(dir, glm::normalize(o->periapsis));
  if (y > 1.0f) y =  1.0f;
  if (y <-1.0f) y = -1.0f;
  float theta = acos(y);
  float l     = dir.x*o->periapsis.y - dir.y*o->periapsis.x;
  if (l < 0.0f) {
    theta = 2.0f * 3.14159f - theta;
  }
  if (theta > 2.0f * 3.14159f) theta -= 2.0f * 3.14159f;
  if (theta < 0.0f)            theta += 2.0f * 3.14159f;

  auto k = sqrtf(u / glm::length(rp) * (1.0f + e));

  auto  normalDirection = glm::normalize(glm::vec2(d.y, -d.x));
  auto  radialDirection = glm::normalize(d);

  p->vel = radialDirection * k * e * sinf(theta)
         + normalDirection * k * (1.0f + e * cosf(theta))
         + focus.get<Position>()->vel;

  if (o->verbose) {
    Log::debug("************");
    Log::debug("dir: %, %",   dir.x, dir.y);
    Log::debug("Rp: %, %",  o->periapsis.x, o->periapsis.y);
    Log::debug("acos: %", glm::dot(dir, glm::normalize(o->periapsis)));
    Log::debug("theta: %",  theta);
    Log::debug("theta: %",  theta);
    Log::debug("vel: %, %", p->vel.x, p->vel.y);
  }

  b->apos = atan2(dir.y, dir.x) * 360.0f/6.28f + 90.0f;

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

void 
renderText(String* s, Transform* x, Color* c, GlTexture* t, GlProgram* p, GlVbo* v) {
  // TODO parameterize
  const float r       = s->size;
  const float offsetx = r;
  const float offsety = r;
  const float padx    = 0.0f;
  const float pady    = r/2.0f;
  const std::string& text    = s->str;

  auto matProjection = game_.entity("camera").get<Projection>()->matrix;
  auto matView       = game_.entity("camera").get<Transform>()->transform;

  matView *= x->transform;
  p->program.use();
  p->program.uniform("mOrtho",      matProjection);
  p->program.uniform("uTexture",    t->texture);
  p->program.uniform("vColor",      c->color);
  p->program.attribute("vPosition", v->vbo, 2, 4*sizeof(float), 0);
  p->program.attribute("vTexture",  v->vbo, 2, 4*sizeof(float), 2*sizeof(float));

  float x_ = 0; 
  float y_ = 0;
  for (size_t i  = 0; i < text.length(); i++) {
    if (text[i] == '\n') {
      y_ += r + pady; 
      x_ = 0;
    } else {
      int nx = text[i] % 16;
      int ny = text[i] / 16;

      // character offset within texture
      auto translated = glm::translate(matView, glm::vec3(x_, y_, 0.0f));
      auto scaled     = glm::scale(translated, glm::vec3(r));

      p->program.uniform  ("offsetX", (float)nx);
      p->program.uniform  ("offsetY", (float)ny);
      p->program.uniform  ("mModel", scaled);

      p->program.execute  (0, 4);

      x_ += r + padx;
    }
  }
}


//------------------------------------------------------------------------------

void removeFromGame(Entity& e) {
  gameQueue_.enqueue( [&] () { game_.remove(e.name()); });
}

Entity& createIndicator(Entity& entity) {
  auto& indicator = game_.entity();
  Log::debug("Created indicator[%]", indicator.name());
  indicator.add<Position>();
  indicator.add<Radius>(entity.get<Radius>()->mag*1.5f);
  indicator.add<Transform>(entity.get<Transform>());
  indicator.add<Color>(0.5f, 0.9f, 0.8f, 0.7f);
  indicator.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                           FragmentShader("assets/gpu/selected.fp"));
  // TODO do we want Geometry, Textures and Programs to be components?
  static const GLfloat pQuad[] = {1.0,  1.0, 1.0, 1.0,  1.0, -1.0, 1.0, 0.0,
                                 -1.0, -1.0, 0.0, 0.0, -1.0,  1.0, 0.0, 1.0};
  indicator.add<GlVbo>(16*sizeof(GLfloat), pQuad);
  return indicator;
}

Entity& createText(Entity& entity) {
  auto& text = game_.entity();
  Log::debug("Created text[%]", text.name());
  text.add<Position>();
  text.add<Track<Position>>(entity.get<Position>());
  text.add<String>(entity.name(), 24.0f);
  text.add<Transform>();
  text.add<Color>(1.0f, 1.0f, 0.0f, 0.8f);
  text.add<GlProgram>(VertexShader  ("assets/gpu/char.vp"), 
                      FragmentShader("assets/gpu/char.fp"));
  // TODO do we want Geometry, Textures and Programs to be components?
  static const GLfloat pQuad[] = {1.0,  1.0, 1.0, 1.0,  1.0, -1.0, 1.0, 0.0,
                                 -1.0, -1.0, 0.0, 0.0, -1.0,  1.0, 0.0, 1.0};
  text.add<GlVbo>(16*sizeof(GLfloat), pQuad);
  unsigned w_, h_;
  text.add<GlTexture>(512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/menlo24.png",    w_, h_).get());
  return text;
}

//------------------------------------------------------------------------------

Entity& createShip(float x, float y, float r) {
  auto& ship = game_.entity();
  Log::debug("Created ship[%]", ship.name());

  ship.add<Time>();
  ship.add<Position>();
  ship.add<Orientation>();
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
  ship.add<Orientation>();
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
  roid.add<Orientation>();
  roid.add<Radius>(r);
  roid.add<HillSphere>(10.0f * r, roid.name());
  roid.add<Pickable>(roid.name());
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

Entity& createMoon(float x, float y, float r) {
  auto& moon = game_.entity();
  Log::debug("Created moon[%]: %", moon.name(), &moon);

  moon.add<Time>();
  moon.add<Position>();
  moon.get<Position>()->pos = glm::vec2(x, y);
  moon.add<Orientation>();
  moon.add<Radius>(r);
  moon.add<HillSphere>(20.0f * r, moon.name());
  moon.add<Pickable>(moon.name());
  moon.add<Mass>(r*r*r);
  moon.add<Transform>();
  moon.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                      FragmentShader("assets/gpu/texture.fp"));

  moon.add<Handler<events::Collision>>([&](events::Collision) {
    //removeFromGame(moon);
  });


  // TODO do we want Geometry, Textures and Programs to be components?
  //
  const GLfloat quad[] = {1.0,  1.0, 1.0, 1.0,
                          1.0, -1.0, 1.0, 0.0,
                         -1.0, -1.0, 0.0, 0.0,
                         -1.0,  1.0, 0.0, 1.0};
  unsigned w_, h_;
  moon.add<GlVbo>(sizeof(quad), quad);
  moon.add<GlTexture>(512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/moon01.png",    w_, h_).get());
  return moon;
}

Entity& createGasPlanet(float x, float y, float r) {
  auto& roid = game_.entity();
  Log::debug("Created gas[%]: %", roid.name(), &roid);

  roid.add<Time>();
  roid.add<Position>();
  roid.add<Orientation>();
  roid.get<Position>()->pos = glm::vec2(x, y);
  roid.add<Radius>(r);
  roid.add<HillSphere>(100.0f * r, roid.name());
  roid.add<Pickable>(roid.name());
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

Entity& createStar(float x, float y, float r) {
  auto& roid = game_.entity();
  Log::debug("Created star[%]: %", roid.name(), &roid);

  roid.add<Time>();
  roid.add<Position>();
  roid.add<Orientation>();
  roid.get<Position>()->pos = glm::vec2(x, y);
  roid.add<Radius>(r);
  roid.add<HillSphere>(1000.0f * r, roid.name());
  roid.add<Pickable>(roid.name());
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
  roid.add<GlTexture>(512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/star01.png",    w_, h_).get());
  return roid;
}


//------------------------------------------------------------------------------

BetaGame::BetaGame(Beta* pMachine) 
  : Beta::State(pMachine) 
{ 
  game_.registerIndex(updateTime);

  game_.registerIndex(&Acceleration::reset);
  game_.registerIndex(orbit);
  game_.registerIndex(&Acceleration::update);
  game_.registerIndex(updatePosition);
  game_.registerIndex(track);
  game_.registerIndex(updateOrientation);

  game_.registerIndex(&Transformations::transform1);
  game_.registerIndex(&Transformations::transform2);
  game_.registerIndex(&Transformations::transform3);
  game_.registerIndex(renderSolid);
  game_.registerIndex(renderTextured);
  game_.registerIndex(renderText);

  game_.registerIndex(&Picker::testPickable);
  game_.registerIndex(&Picker::testHillSphere);
  game_.registerIndex(&CollisionDetector::update);

  game_.registerEvent<events::Collision>();

  auto& star = createStar(0.0f, 0.0f, 5000.0f);

  auto   e = 0.0f;
  auto  rp = 1500.0f;
  auto  ra = rp * (1.0f + e)/(1.0f - e);
  auto& gas = createGasPlanet(0.0f, 5000.0f + ra, 500.0f);
  gas.add<Orbit>(glm::vec2(0.0f, -ra), e, 0.0f, gas.name(), star.name());
  createText(gas);

  auto& cam = game_.entity("camera");
  cam.add<Projection>(roidrage::Display::getWidth(), roidrage::Display::getHeight(), 0.07f);
  cam.add<Time>();
  cam.add<Position>();
  cam.add<Transform>();

  for (unsigned i = 0; i < 1; i++) {
    auto& ship = createShip(100.0f, gas.get<Position>()->pos.y + 500.0f + 100.0f, 20.0f);
    ship.add<Orbit>(glm::vec2(0.0f, 100.000f), 0.0f, 0.0f, ship.name(), gas.name());
    createText(ship);
  }

  /*
  for (unsigned i = 0; i < 5; i++) {
    auto& ship = createBoid(100.0f + i*10.0f, i*10.0f, 80.0f);
    ship.add<Orbit>(600.0f + rand() % 100, ship.get<Position>()->pos, ship.name(), gas.name());
    createText(ship);
  }

  */
  for (unsigned i = 0; i < 5; i++) {
    auto& roid = createRoid(6000.0f - (rand() % 200), 6000.0f - (rand() % 200), 40.0f + rand() % 40);
    roid.add<Orbit>(glm::vec2(1000.0f + rand() % 200, 1000.0f + rand() % 200), 0.0f, 0.0f, roid.name(), star.name());
    createText(roid);
  }

  for (unsigned i = 0; i < 1; i++) {
    auto& moon = createMoon(0.0f, gas.get<Position>()->pos.y + 500.0f + 1000.0f, 100.0f);
    moon.add<Orbit>(glm::vec2(0.0f, 1000.000f), 0.0f, 0.0f, moon.name(), gas.name());
    createText(moon);
  }

  timerReactor_.setPeriodic(std::chrono::seconds(5), [&] () {
    gameQueue_.enqueue( [&] () {
    });
  });
}

//------------------------------------------------------------------------------

void 
BetaGame::onEvent(Tick tick) {
  static Acceleration       acceleration;
  static Transformations    transforms;
  static CollisionDetector collisions;

  game_.exec(acceleration, &Acceleration::reset); 

  // Begin FSM stuff
  game_.exec(orbit); 

  //game_.exec(acceleration, &Acceleration::update); 
  game_.exec(updateTime); 
  game_.exec(updatePosition); 
  game_.exec(track); 
  game_.exec(updateOrientation); 

  game_.exec(transforms, &Transformations::transform1); 
  game_.exec(transforms, &Transformations::transform2); 
  transforms.exec();

  game_.exec(transforms, &Transformations::transform3); 

  game_.exec(collisions, &CollisionDetector::update); 

  glClearColor(0.07f, 0.07f, 0.13f, 0.01f);
  glClear(GL_COLOR_BUFFER_BIT);

  game_.exec(renderSolid); 
  game_.exec(renderTextured); 
  game_.exec(renderText); 

  // execute actions 'scheduled' for this tick
  while (gameQueue_.work());
}

//------------------------------------------------------------------------------
// TODO definitely a better way for this - perhaps a map of keys and lambdas?
void 
BetaGame::onEvent(GlfwKey key) {
  float scrollSpeed = 2000.0f / zoom;

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
BetaGame::onEvent(GlfwMouseMove mouse) {
}

//------------------------------------------------------------------------------

void 
BetaGame::onEvent(GlfwMouseScroll mouse) {
  float zoomSensitivity = 0.01f * zoom;
  zoom += mouse.y * zoomSensitivity;
  zoom = std::min(zoom,  5.0f);
  zoom = std::max(zoom,  0.001f);
  auto& cam = game_.entity("camera");
  cam.replace<Projection>(roidrage::Display::getWidth(), roidrage::Display::getHeight(), zoom); 
  game_.entity("camera").get<Position>()->vel = glm::vec2(0.0f, 0.0f);
}

//------------------------------------------------------------------------------
// this is a hot mess
void 
BetaGame::onEvent(GlfwMouseButton mouse) {
  static Picker    picker;

  // TODO this should be an inverse transform...
  auto& cam = game_.entity("camera");
  auto position = glm::vec2(mouse.x - roidrage::Display::getWidth()/2.0f, 
                            mouse.y - roidrage::Display::getHeight()/2.0f);
  position /= zoom;
  position -= cam.get<Position>()->pos;

  if (mouse.button == GLFW_MOUSE_BUTTON_LEFT) {
    if (mouse.action == GLFW_PRESS) {
      picker.startBox(position);

      if (!mouse.mods & GLFW_MOD_SHIFT) {
        Selection::clear(); 
        //if (cam.has<Track<Position>>()) {
        //  cam.rem<Track<Position>>();
        //}
      }
    }
    if (mouse.action == GLFW_RELEASE) {
      picker.updateBox(position);
      game_.exec(picker, &Picker::testPickable); 
      auto candidates = picker.query();
      picker.reset();

      candidates.sort([](Entity* a, Entity* b) -> bool {
        return a->get<Radius>()->mag < b->get<Radius>()->mag;
      });

      if (!candidates.empty()) {
        if (picker.isRange()) {
          for (auto e : candidates) {
            Selection::toggle(*e);
          }
        } else {
          auto e = candidates.front();
          Selection::toggle(*e);
          //cam.addOrReplace<Track<Position>>(e->get<Position>());
        }
      }   
    }
  }

  else if (mouse.button == GLFW_MOUSE_BUTTON_RIGHT && mouse.action == GLFW_PRESS) {
    picker.startBox(position);
    game_.exec(picker, &Picker::testHillSphere); 
    auto candidates = picker.query();
    candidates.sort([&](Entity* a, Entity* b) -> bool {
      float ra = glm::length(position - a->get<Position>()->pos);
      float rb = glm::length(position - b->get<Position>()->pos);
      return ra < rb;
    });

    for (auto c : candidates) {
      float rc = glm::length(position - c->get<Position>()->pos);
      Log::debug("candidate [%]: r=%", c->name(), rc);
    }
    for (auto kv : Selection::selected) {
      auto s = kv.first;
      //for (auto h : candidates) {
      auto h = candidates.front();
      if (!candidates.empty() && h != s) {
        auto r1 = h->get<Position>()->pos - s->get<Position>()->pos;
        //if(glm::length(s->get<Position>()->vel) > 0.0f) {
        auto r2 = -1.2f * glm::length(h->get<Position>()->pos - position) * glm::normalize(r1);
        glm::vec2 periapsis;
        glm::vec2 apoapsis;
        if (glm::length(r1) < glm::length(r2)) {
          periapsis = r1;
          apoapsis  = r2;
        } else {
          periapsis = r2;
          apoapsis  = r1;
        }
        auto  e = std::abs((glm::length(apoapsis) - glm::length(periapsis))
                         / (glm::length(apoapsis) + glm::length(periapsis)));
        Log::debug("new orbit - Ra: % (%,%), Rp: % (%,%), e: %", 
                   glm::length(apoapsis), apoapsis.x, apoapsis.y, glm::length(periapsis), periapsis.x, periapsis.y, e);
        s->addOrReplace<Orbit>(periapsis, e, 0.0f, s->name(), h->name());
      }
    }
    picker.reset();
  }
}

//------------------------------------------------------------------------------
