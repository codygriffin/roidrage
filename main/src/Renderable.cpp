//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "RoidRage.h"
#include "Renderable.h"
#include "OrthoCamera.h"
#include "Program.h"
#include "VertexBufferObject.h"
#include "Texture.h"
#include "Params.h"

using namespace roidrage;
using namespace pronghorn;

//------------------------------------------------------------------------------

void 
Renderable::render(OrthoCamera* pCam) {
  preRender(pCam);
  onRender(pCam);
  postRender();
}

//------------------------------------------------------------------------------

Quad::Quad(Texture*      pTexture, 
     Program*            pProgram,
     VertexBufferObject* pVbo) {
  // Assign our programs, textures and buffers
  getRenderPass().pProgram = pProgram;
  getRenderPass().pVbo.push_back(pVbo);
  if (pTexture) {
    getRenderPass().pTex.push_back(pTexture);
  }
}

void  Quad::preRender(OrthoCamera* pCam) {
  // Use the assigned program
  getRenderPass().pProgram->use();

  // Assign some uniforms
  getRenderPass().pProgram->uniform  ("mOrtho", pCam->getOrthoMatrix());
  getRenderPass().pProgram->uniform  ("mModel", getRenderPass().modelMatrix);
  getRenderPass().pProgram->uniform  ("vColor", getRenderPass().color);

  // If we have a texture, assign it
  if (!getRenderPass().pTex.empty()) {
    getRenderPass().pProgram->uniform  ("uTexture",  *getRenderPass().pTex[0]);
  }

  // Setup our attribtues
  getRenderPass().pProgram->attribute("vPosition", *getRenderPass().pVbo[0], 2, 4*sizeof(float), 0);
  getRenderPass().pProgram->attribute("vTexture",  *getRenderPass().pVbo[0], 2, 4*sizeof(float), 2*sizeof(float));
};

void  Quad::onRender(OrthoCamera* pCam) {
  // Run our program
  getRenderPass().pProgram->execute  (0, 4);
};

//------------------------------------------------------------------------------

SolidQuad::SolidQuad(float x, float y, float w, float h) 
  : Quad(0, pRoidRage->pSolidProgram.get(), pRoidRage->pQuadVboPos.get()) {
  getRenderPass().modelMatrix = glm::scale(
    glm::translate(
      glm::mat4(),
      glm::vec3(x + w * 0.5f, y + h * 0.5f, 0.0f)
    ),
    glm::vec3(w * 0.5f, h * 0.5f, 1.0f)
  );

}

void 
SolidQuad::preRender(OrthoCamera* pCam) {
  getRenderPass().pProgram->use();

  getRenderPass().pProgram->uniform  ("mOrtho", pCam->getOverlayMatrix());
  getRenderPass().pProgram->uniform  ("mModel", getRenderPass().modelMatrix);
  getRenderPass().pProgram->uniform  ("vColor", getRenderPass().color);

  getRenderPass().pProgram->attribute("vPosition", *getRenderPass().pVbo[0], 2, 4*sizeof(float), 0);
  getRenderPass().pProgram->attribute("vTexture",  *getRenderPass().pVbo[0], 2, 4*sizeof(float), 2*sizeof(float));
}

//------------------------------------------------------------------------------

ShadedQuad::ShadedQuad(float x, float y, float w, float h, Program* pProgram) 
  : Quad(0, pProgram, pRoidRage->pQuadVboPos.get()) {
  getRenderPass().modelMatrix = glm::scale(
    glm::translate(
      glm::mat4(),
      glm::vec3(x + w * 0.5f, y + h * 0.5f, 0.0f)
    ),
    glm::vec3(w * 0.5f, h * 0.5f, 1.0f)
  );

}

void 
ShadedQuad::preRender(OrthoCamera* pCam) {
  getRenderPass().pProgram->use();

  getRenderPass().pProgram->uniform  ("mOrtho", pCam->getOrthoMatrix());
  getRenderPass().pProgram->uniform  ("mModel", getRenderPass().modelMatrix);
  getRenderPass().pProgram->uniform  ("vColor", getRenderPass().color);

  getRenderPass().pProgram->attribute("vPosition", *getRenderPass().pVbo[0], 2, 4*sizeof(float), 0);
  getRenderPass().pProgram->attribute("vTexture",  *getRenderPass().pVbo[0], 2, 4*sizeof(float), 2*sizeof(float));
}

//------------------------------------------------------------------------------
