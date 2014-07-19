//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_RENDERABLE_H
#define INCLUDED_RENDERABLE_H

#ifndef INCLUDED_GLM_HPP
#include "glm.hpp"
#define INCLUDED_GLM_HPP
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

#ifndef INCLUDED_PARAMS
#include "Params.h"
#endif

//------------------------------------------------------------------------------

namespace pronghorn { 
  struct Texture;
  struct Program;
  struct VertexBufferObject;
}

namespace roidrage { 

//------------------------------------------------------------------------------

struct OrthoCamera;
struct RenderPass {
  bool                                        visible;
  glm::mat4                                   projectionMatrix;
  glm::mat4                                   modelMatrix;

  glm::vec4                                   color;
  pronghorn::Program*                         pProgram;
  std::vector<pronghorn::VertexBufferObject*> pVbo;
  std::vector<pronghorn::Texture*>            pTex;
};


struct Renderable : public Component<Renderable> {
  void render(OrthoCamera* pCam);

  inline RenderPass& getRenderPass() {
    return renderPass_;
  }

protected:
  virtual void  preRender(OrthoCamera* pCam) {};
  virtual void   onRender(OrthoCamera* pCam) = 0;
  virtual void postRender() {};

  RenderPass renderPass_;
};


struct Quad : public Renderable {
  Quad(pronghorn::Texture*            pTexture, 
       pronghorn::Program*            pProgram,
       pronghorn::VertexBufferObject* pVbo); 

  void  preRender(OrthoCamera* pCam);
  void  onRender(OrthoCamera* pCam);
};

struct SolidQuad : public Quad {
  SolidQuad(float x, float y, float w, float h);
  void preRender(OrthoCamera* pCam);
};

struct ShadedQuad : public Quad {
  ShadedQuad(float x, float y, float w, float h, pronghorn::Program* pProgram);
  void preRender(OrthoCamera* pCam);
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
