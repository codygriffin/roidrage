//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "BetaGame.h"

#include "Viewport.h"
#include "Shader.h"
#include "Program.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "VertexBufferObject.h"

#include "AssetManager.h"
#include "ResourceManager.h"
#include "Display.h"

#include "Entity.h"
#include "System.h"

#include "Log.h"
#include "WorkQueue.h"

#include "Panel.h"
#include "Button.h"
#include "Label.h"

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

static System               game_;
static corvid::WorkQueue    gameQueue_;
static const float          quad[] = {-1.0f, -1.0f, 0.0f, 1.0f,  
                                       1.0f, -1.0f, 1.0f, 1.0f,
                                       1.0f,  1.0f, 1.0f, 0.0f, 
                                      -1.0f,  1.0f, 0.0f, 0.0f};

//------------------------------------------------------------------------------
// Big hacks - these should be cleaned up and broken out into proper classes 
#include "BetaComponents.h"
#include "BetaSystems.h"
#include "FlightPlan.h" 

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
  indicator.add<GlVbo>(16*sizeof(GLfloat), quad);
  return indicator;
}

Entity& createText(Entity& entity, float x = 0.0f, float y = 0.0f) {
  auto& text = game_.entity();
  Log::debug("Created text[%]", text.name());
  text.add<Position>();
  text.get<Position>()->pos = glm::vec2(x, y);
  text.add<Track>(entity.get<Position>(), 
                  glm::vec2(-entity.get<Radius>()->mag, 
                             entity.get<Radius>()->mag + 40.0f));
  text.add<Transform>();
  text.add<String>(entity.name(), 40.0f);
  text.add<Color>(1.0f, 1.0f, 1.0f, 0.8f);
  text.add<GlProgram>(VertexShader  ("assets/gpu/char.vp"), 
                      FragmentShader("assets/gpu/char.fp"));
  // TODO do we want Geometry, Textures and Programs to be components?
  text.add<GlVbo>(16*sizeof(GLfloat), quad);
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
  ship.add<Moveable>();
  ship.add<Transform>();
  ship.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                      FragmentShader("assets/gpu/texture.fp"));
  // TODO do we want Geometry, Textures and Programs to be components?
  //
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
  ship.add<Moveable>();
  ship.add<Transform>();
  ship.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                      FragmentShader("assets/gpu/texture.fp"));
  // TODO do we want Geometry, Textures and Programs to be components?
  //
  unsigned w_, h_;
  ship.add<GlVbo>(sizeof(quad), quad);
  ship.add<GlTexture>(512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/boid01.png", w_, h_).get());
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
  roid.add<Moveable>();
  roid.add<Mass>(r*r*r);
  roid.add<Transform>();
  roid.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                      FragmentShader("assets/gpu/texture.fp"));

  roid.add<Handler<events::Collision>>([&](events::Collision) {
    //removeFromGame(roid);
  });


  // TODO do we want Geometry, Textures and Programs to be components?
  //
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
  moon.add<Mass>(r*r*r*500.0f);
  moon.add<Transform>();
  moon.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                      FragmentShader("assets/gpu/texture.fp"));

  moon.add<Handler<events::Collision>>([&](events::Collision) {
    //removeFromGame(moon);
  });


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
  roid.add<Mass>(r*r*r*100.0f);
  roid.add<Transform>();
  roid.add<GlProgram>(VertexShader  ("assets/gpu/transform.vp"), 
                      FragmentShader("assets/gpu/texture.fp"));


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


  unsigned w_, h_;
  roid.add<GlVbo>(sizeof(quad), quad);
  roid.add<GlTexture>(512, 512, Texture::RGBA, AssetManager::loadBitmap("assets/png/star01.png",    w_, h_).get());
  return roid;
}

//------------------------------------------------------------------------------

ui::Panel* buildPlanetMenu(Entity& planet) {
  auto panel = new ui::Panel(glm::vec4(100.0f, 100.0f, 400.0f, 400.0f),
                             glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
  
  panel->add(new roidrage::Label(planet.name(), 24.0f)); 
  panel->add(new roidrage::Button("spawn ship", 
    [&](beta::GlfwMouseButton mouse) {
      if (mouse.button == GLFW_MOUSE_BUTTON_LEFT && 
         mouse.action == GLFW_PRESS) {
        auto pos = planet.get<Position>()->pos;
        auto r   = 1.0f + (rand() % 100)/100.0f; 
        auto& ship = createShip(r * ~planet + pos.x, pos.y, 100.0f);
        park(ship, planet, r);
        createText(ship);
      }
    }
  , 24.0f));
  panel->add(new roidrage::Button("spawn roid", 
    [&](beta::GlfwMouseButton mouse) {
      if (mouse.button == GLFW_MOUSE_BUTTON_LEFT && 
         mouse.action == GLFW_PRESS) {
        auto pos = planet.get<Position>()->pos;
        auto r   = 2.0f + (rand() % 100)/100.0f; 
        auto& ship = createRoid(r * ~planet + pos.x, pos.y, 100.0f);
        park(ship, planet, r);
        createText(ship);
      }
    }
  , 24.0f));
  panel->add(new roidrage::Button("spawn moon", 
    [&](beta::GlfwMouseButton mouse) {
      if (mouse.button == GLFW_MOUSE_BUTTON_LEFT && 
         mouse.action == GLFW_PRESS) {
        auto pos = planet.get<Position>()->pos;
        auto r   = 3.0f + (rand() % 100)/100.0f; 
        auto& ship = createMoon(r * ~planet + pos.x, pos.y, 100.0f);
        park(ship, planet, r);
        createText(ship);
      }
    }
  , 24.0f));
  panel->add(new roidrage::Button("spawn planet", 
    [&](beta::GlfwMouseButton mouse) {
      if (mouse.button == GLFW_MOUSE_BUTTON_LEFT && 
         mouse.action == GLFW_PRESS) {
        auto pos = planet.get<Position>()->pos;
        auto r   = 4.0f + (rand() % 1000)/100.0f; 
        auto& ship = createGasPlanet(r * ~planet + pos.x, pos.y, 300.0f + rand() % 800);
        park(ship, planet, r);
        createText(ship);
      }
    }
  , 24.0f));
  return panel;
}

BetaGame::BetaGame(Beta* pMachine) 
  : Beta::State(pMachine) 
  , layout_(0.0f, 0.0f, roidrage::Display::getWidth(), roidrage::Display::getHeight())
{ 
  game_.registerIndex(updateTime);

  game_.registerIndex(&Acceleration::reset);
  game_.registerIndex(flightGoals);
  game_.registerIndex(flightControl);
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

  auto& cam = game_.entity("camera");
  cam.add<Projection>(roidrage::Display::getWidth(), roidrage::Display::getHeight(), 0.1);
  cam.add<Time>();
  cam.add<Position>();
  //cam.add<Orientation>();
  cam.add<Transform>();

  auto& overlay = game_.entity("overlay");
  overlay.add<Projection>(roidrage::Display::getWidth(), roidrage::Display::getHeight(), 1.0);

  auto& star = createStar(0.0f, 0.0f, 5000.0f);

  auto& gas = createGasPlanet(0.0f, ~star*1.5f, 500.0f);
  park(gas, star);
  createText(gas);

  auto& gas2 = createGasPlanet(0.0f, -~star*3.5f, 700.0f);
  park(gas2, star, 3.5f);
  createText(gas2);

  for (unsigned i = 0; i < 1; i++) {
    auto& ship = createShip(gas.get<Position>()->pos.x + ~gas*3.0f, gas.get<Position>()->pos.y, 20.0f);
    park(ship, gas, 3.0f); 
    createText(ship);
  }

  auto& moon = createMoon(0.0f, gas.get<Position>()->pos.y + ~gas*1.5f, 100.0f);
  park(moon, gas);
  createText(moon);

  auto& moon2 = createMoon(0.0f, gas2.get<Position>()->pos.y + ~gas2*1.5f, 100.0f);
  park(moon2, gas2);
  createText(moon2);

  timerReactor_.setPeriodic(std::chrono::seconds(5), [&] () {
    gameQueue_.enqueue( [&] () {
    });
  });

  layout_.clear();
  layout_.add(buildPlanetMenu(gas));
  layout_.layout();
}

//------------------------------------------------------------------------------

void 
BetaGame::onEvent(Tick tick) {
  static Acceleration       acceleration;
  static Transformations    transforms;
  static CollisionDetector collisions;

  game_.exec(acceleration, &Acceleration::reset); 

  // Begin FSM stuff
  game_.exec(flightGoals); 
  game_.exec(flightControl); 

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

  auto projection = glm::ortho(0.0f, 
                               roidrage::Display::getWidth(), 
                              -roidrage::Display::getHeight(), 
                               0.0f, 
                              -1.0f,  1.0f);
  layout_.render(projection, glm::vec2());

  // execute actions 'scheduled' for this tick
  while (gameQueue_.work());
}

//------------------------------------------------------------------------------
// TODO definitely a better way for this - perhaps a map of keys and lambdas?
void 
BetaGame::onEvent(GlfwKey key) {
  float scrollSpeed = 2000.0f / zoom;
  float rotateSpeed = 20.0f;

  auto& cam = game_.entity("camera");

  auto up    = glm::vec2(0.0f, -scrollSpeed);
  auto right = glm::vec2(-scrollSpeed, 0.0f);
  
  // Convert into camera-local
  float theta = atan2(cam.get<Transform>()->transform[0][0],
                      cam.get<Transform>()->transform[0][1])
              - 3.14156f/2.0f;
  up    = glm::rotate(up,    theta);
  right = glm::rotate(right, theta);
  // need to rotate to match camera

  // Factor out all of the navigation stuff 
  // Doesn't work too well if you are scrolling and moving 
  // (gets 'stuck')
  switch (key.key) {
    case GLFW_KEY_W:
      if (key.action == GLFW_PRESS) {
        cam.get<Position>()->vel += up;
      }
      if (key.action == GLFW_RELEASE) {
        cam.get<Position>()->vel -= up;
      }
    break;
    case GLFW_KEY_A:
      if (key.action == GLFW_PRESS) {
        cam.get<Position>()->vel += -right;
      }
      if (key.action == GLFW_RELEASE) {
        cam.get<Position>()->vel -= -right;
      }
    break;
    case GLFW_KEY_S:
      if (key.action == GLFW_PRESS) {
        cam.get<Position>()->vel += -up;
      }
      if (key.action == GLFW_RELEASE) {
        cam.get<Position>()->vel -= -up;
      }
    break;
    case GLFW_KEY_D:
      if (key.action == GLFW_PRESS) {
        cam.get<Position>()->vel += right;
      }
      if (key.action == GLFW_RELEASE) {
        cam.get<Position>()->vel -= right;
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
  cam.replace<Projection>(roidrage::Display::getWidth(), 
                          roidrage::Display::getHeight(), 
                          zoom); 
  game_.entity("camera").get<Position>()->vel = glm::vec2(0.0f, 0.0f);
}

//------------------------------------------------------------------------------
// this is a hot mess
void 
BetaGame::onEvent(GlfwMouseButton mouse) {
  static Picker    picker;

  if (layout_.event(mouse)) return;

  // unproject camera to get world coordinates 
  auto& cam = game_.entity("camera");
  auto camModel  = cam.get<Transform>()->transform;
  auto camProj   = cam.get<Projection>()->matrix;
  auto modelView = camProj * camModel;
  auto mousePos  = glm::vec3(mouse.x,
                             roidrage::Display::getHeight() - mouse.y,
                             0.0f);
  auto world3    = glm::unProject(mousePos, camModel, camProj, gl::Viewport::get());
  auto position  = glm::vec2(world3);

  if (mouse.button == GLFW_MOUSE_BUTTON_LEFT) {
    if (mouse.action == GLFW_PRESS) {

      picker.startBox(position);

      if (!mouse.mods & GLFW_MOD_SHIFT) {
        Selection::clear(); 
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
          layout_.clear();
          layout_.add(buildPlanetMenu(*e));
          layout_.layout();
        }
      }   
    }
  }

  else if (mouse.button == GLFW_MOUSE_BUTTON_RIGHT && mouse.action == GLFW_PRESS) {
    picker.startBox(position);
    game_.exec(picker, &Picker::testPickable); 
    auto candidates = picker.query();
    Log::debug("picking moveTo candidate");

    // prioritize destination based on distance from click
    // TODO: this isn't very good
    candidates.sort([=](Entity* a, Entity* b) -> bool {
      float ra = glm::length(position - a->get<Position>()->pos);
      float rb = glm::length(position - b->get<Position>()->pos);
      return ra < rb;
    });


    // move selected entities to destination
    for (auto kv : Selection::selected) {
      auto s = kv.first;
      //for (auto h : candidates) {
      auto h = candidates.front();
      Log::debug("do we have a candidate");
      if (!candidates.empty() && h != s) {
      //if (!candidates.empty() && h != s && s->has<Moveable>()) {
        Log::debug("moving to %", h->name());
        moveTo(*s, *h);
      }
    }
    picker.reset();
  }
}

//------------------------------------------------------------------------------
