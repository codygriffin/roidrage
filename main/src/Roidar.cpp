//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Roidar.h"
#include "RoidRage.h"
#include "OrthoCamera.h"
#include "Program.h"
#include "VertexBufferObject.h"
#include "Systems.h"
#include "Params.h"

//------------------------------------------------------------------------------

using namespace roidrage;

//------------------------------------------------------------------------------

Roidar::Roidar()
{
  getRenderPass().pProgram = pRoidRage->pOverlayProgram.get();
  getRenderPass().pVbo.push_back(pRoidRage->pOverlayVbo.get());
}

//------------------------------------------------------------------------------

void
Roidar::preRender(OrthoCamera* pCam)
{
  getRenderPass().color  = glm::vec4(0.5f, 0.5f, 0.2f, 0.2f);
  getRenderPass().pProgram->use();

  getRenderPass().projectionMatrix = glm::ortho(
                     -(float)pRoidRage->width/2, 
                      (float)pRoidRage->width/2, 
                      (float)pRoidRage->height/2, 
                     -(float)pRoidRage->height/2, 
                     -1.0f, 1.0f);

  getRenderPass().pProgram->uniform  ("mModel", getRenderPass().modelMatrix);
  getRenderPass().pProgram->uniform  ("mOrtho", getRenderPass().projectionMatrix);

  getRenderPass().pProgram->uniform  ("vColor", getRenderPass().color);
  getRenderPass().pProgram->attribute("vPosition", *getRenderPass().pVbo[0], 2, 0, 0);
}

//------------------------------------------------------------------------------

void  Roidar::onRender(OrthoCamera* pCam) {
  for (int i = 0; i < 12; i++) {
    getRenderPass().modelMatrix = glm::scale(
                          glm::rotate(
                            glm::translate(
                              glm::mat4(), 
                              glm::vec3(0.0f, 0.0f, 0.0f)
                            ),
                            30.0f * -i, 
                            glm::vec3(0.0f, 0.0f, 1.0f)
                          ),
                          glm::vec3(pRoidRage->radar[i], pRoidRage->radar[i], 1.0f)
                        );
    getRenderPass().pProgram->uniform  ("mModel", getRenderPass().modelMatrix);
    getRenderPass().pProgram->execute  (0, 6);
  }
};

//------------------------------------------------------------------------------
