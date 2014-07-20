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
#include "Systems.h"

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

void 
transform(Transform* x, Position* p, Radius* r) {
  glm::vec2 pos = p->pos;
  float     mag = r->mag;

  // Set our position, rotation and scale according
  // to the position, radius and angular position.
  x->transform = glm::scale(
    glm::translate(
      glm::mat4(),
      glm::vec3(pos.x, pos.y, 0.0f)
    ),
    glm::vec3(mag*1.0f)
  );

  x->transform = glm::rotate(
    x->transform, 
    glm::radians(p->apos),
    glm::vec3(0.0f,0.0f,1.0f)
  );
}

//------------------------------------------------------------------------------

glm::vec2 Selectable::position;
void 
testSelected(Selectable* s, Position* p, Radius* r) {
  // Displacement is the vector pointing from this entity to the 
  // other entity, with a length corresponding to the distance
  auto displacement = p->pos - Selectable::position;
  Log::debug("testing @(%,%) for position (%,%) radius %", 
             Selectable::position.x, Selectable::position.y,
             p->pos.x, p->pos.y,
             r->mag);

  if (r->mag > glm::length(displacement)) { 
    if (!s->selected) {
      s->selected = true;
      auto& indicator = game_.entity();
      s->indicator = indicator.name();
      Log::debug("Created indicator[%]", indicator.name());
      indicator.add<Position>();
      indicator.get<Position>()->pos.x = 0;
      indicator.get<Position>()->pos.y = 0;
      indicator.add<Radius>(90.0f);
      indicator.add<Transform>();
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
  game_.registerIndex(renderSolid);
  game_.registerIndex(renderTextured);
  game_.registerIndex(transform);
  game_.registerIndex(testSelected);

  auto& ship = game_.entity();
  Log::debug("Created ship[%]", ship.name());
  ship.add<Position>();
  ship.get<Position>()->pos.x = 0;
  ship.get<Position>()->pos.y = 0;
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

  //ship.add<Quad>(pRoidRage->pSpaceshipTex[1].get(), 
  //               pRoidRage->pProgram.get(), 
  //               pRoidRage->pQuadVboPos.get());

  timerReactor_.setPeriodic(std::chrono::seconds(5), [&] () {
    gameQueue_.enqueue( [&] () {
      Log::debug("<Every 5 Seconds>");
    });
  });
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(Tick tick) {

  game_.visit(transform); 

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
    game_.visit(testSelected); 
  }
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(AndroidBack back) {
  getMachine()->transition<RoidRageMenu>(true);
}

//------------------------------------------------------------------------------
