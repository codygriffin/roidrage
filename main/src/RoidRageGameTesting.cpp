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

void 
transform(Quad* pRenderable, Position* p, Radius* r) {
  glm::vec2 pos = p->pos;
  float     mag = r->mag;

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

void 
testSelected(Selectable* s, Position* p, Radius* r) {
}

//------------------------------------------------------------------------------

void render(Quad* pRenderable) {
  pRenderable->render(RenderState::pCam_.get());
}

//------------------------------------------------------------------------------

RoidRageGameTesting::RoidRageGameTesting(RoidRage* pMachine) 
  : RoidRage::State(pMachine) 
{ 
  //game_.registerIndex(render);
  game_.registerIndex<Quad>();
  game_.registerIndex<Quad, Position, Radius>();
  game_.registerIndex<Selectable, Position, Radius>();

  auto ship = game_.entity();
  Log::debug("Created ship[%]", ship.name());
  ship.add<Position>();
  ship.get<Position>()->pos.x = 0;
  ship.get<Position>()->pos.y = 0;
  ship.add<Radius>(90.0f);
  ship.add<Selectable>(false);
  ship.add<Quad>(pRoidRage->pSpaceshipTex[1].get(), 
                 pRoidRage->pProgram.get(), 
                 pRoidRage->pQuadVboPos.get());

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
  game_.visit(testSelected); 

  pRoidRage->ortho = RenderState::pCam_->getOrthoMatrix(); 
  glClearColor(0.07f, 0.07f, 0.13f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  game_.visit(render); 

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
}

//------------------------------------------------------------------------------

void 
RoidRageGameTesting::onEvent(AndroidBack back) {
  getMachine()->transition<RoidRageMenu>(true);
}

//------------------------------------------------------------------------------
