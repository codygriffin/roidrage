//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Background.h"
#include "RoidRage.h"
#include "OrthoCamera.h"

#include "Log.h"

//------------------------------------------------------------------------------

using namespace roidrage;
using namespace pronghorn;

//------------------------------------------------------------------------------

Background::Background()
: Quad      (0, pRoidRage->pBgProgram.get(), pRoidRage->pQuadVboPos.get())
, TimeDelta ()
{
  getRenderPass().color  = glm::vec4(0.2f, 0.4f, 0.1f, 1.0f);
  getRenderPass().color *= 0.5f;

  getRenderPass().modelMatrix = glm::scale(
    glm::mat4(),
    glm::vec3(Position::max, Position::max, 1.0f)
  );
}

//------------------------------------------------------------------------------

Background::~Background() {
}

//------------------------------------------------------------------------------

void  
Background::preRender(OrthoCamera* pCam) {
  getRenderPass().pProgram->use();

  getRenderPass().pProgram->uniform  ("mOrtho", pCam->getOrthoMatrix());
  getRenderPass().pProgram->uniform  ("mModel", getRenderPass().modelMatrix);
  getRenderPass().pProgram->uniform  ("vColor", getRenderPass().color);
  //getRenderPass().pProgram->uniform  ("depth",  1.0f);

  getRenderPass().pProgram->attribute("vPosition", *getRenderPass().pVbo[0], 2, 4*sizeof(GLfloat), 0);
  getRenderPass().pProgram->attribute("vTexture",  *getRenderPass().pVbo[0], 2, 4*sizeof(GLfloat), 2*sizeof(GLfloat));

  // TODO: fold Background/Overlay into the actual population
  static double bgtime = 0;
  bgtime += double(ms);
  int t = 16000.0 * sin(bgtime/16000.0);
  if (bgtime > 6.28*16000.0) bgtime = 0.0;

  getRenderPass().pProgram->uniform  ("time",  t);
};

//------------------------------------------------------------------------------
