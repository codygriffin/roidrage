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

//------------------------------------------------------------------------------
// Big hacks - these should be cleaned up and broken out into proper classes 
#include "BetaComponents.h"
#include "BetaSystems.h"

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
