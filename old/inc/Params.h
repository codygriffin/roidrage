//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_PARAMS_H
#define INCLUDED_PARAMS_H

#include "glm.hpp"
#include "matrix_transform.hpp"
#include "vector_angle.hpp"
#include <vector>

#include "TimeMs.h"
#include "Shader.h"
#include "Program.h"
#include "Texture.h"
#include "VertexBufferObject.h"

#include "Component.h"

//------------------------------------------------------------------------------

namespace roidrage { 

struct Glow {
  float glow;
};

struct TimeDelta : public boson::Component<TimeDelta> {
  int ms;
};

struct Position : public boson::Component<Position> {
  glm::vec2 pos;
  float     apos;

  glm::vec2 vel;
  float     avel;

  float     maxVel;

  glm::vec2 acc;
  float     aacc;

  float     distance;

  Position() 
  : apos(0.0f)
  , avel(0.0f)
  , maxVel(3.0f)
  , aacc(0.0f)
  , distance(0.0f)
  {}

  static constexpr float max         =  4000.0f;
  static constexpr float min         = -4000.0f;
  static constexpr float maxDistance =  8000.0f;
};

struct Orientation : public boson::Component<Orientation> {
  float     apos;
  float     avel;
  float     aacc;

  Orientation() 
  : apos(0.0f)
  , avel(0.0f)
  , aacc(0.0f)
  {}
};

struct Color  : public boson::Component<Color> {
  Color (float r=0.0f, float g=1.0f, float b=0.0f, float a=0.5f) 
    : color(r,g,b,a) {
  }
  glm::vec4 color;
};

struct Charge {
  static constexpr float unit = -3.6e-8f;
  Charge(float r) 
    : mag(r) {
  }
  float mag;
};

struct Mass : public boson::Component<Mass> {
  static constexpr float unit = 3.6e-8f;
  Mass(float r) 
    : mag(r) {
  }
  float mag;
};

struct Radius  : public boson::Component<Radius> {
  Radius(float r) 
    : mag(r) {
  }

  float mag;
};

struct Collision {
  Collision(glm::vec2 d, float b)
    : displacement(d)
    , boundary(b) {
  } 
  glm::vec2 displacement;
  float     boundary;
};

struct Expiration {
  bool isExpired() {
    return ageMs > expiresInMs;
  }

  float expiration() {
    return (float)ageMs/(float)expiresInMs;
  }

  void age(unsigned ms) {
    ageMs += ms;
  }

  void reset() {
    ageMs = 0;
  }

  Expiration(unsigned e) 
    : expiresInMs(e) {
    reset();
  }

  unsigned expiresInMs;
  unsigned ageMs;
};

struct RateLimiter {
  RateLimiter(float rate, float burst, float buffer=0.0f)
  : rate_(rate)
  , burst_(burst)
  , buffer_(buffer) {
  }

  bool isRateLimited() const {
    refresh();  
    return buffer_ <= 0.0f;
  }

  bool consume() const {
    bool isLimited = isRateLimited();
    if (!isLimited) {
      buffer_ -= 1.0f;
    }
    return isLimited;
  }

  void refresh() const {
    float addition = (time_.mark() / (rate_ * 1000.0f));
    buffer_  = std::min(buffer_ + addition , burst_);
  }

private:
  float            rate_;
  float            burst_;
  mutable float    buffer_;
  mutable TimeMs   time_;
};

struct Thrust {
  static constexpr float unit = -9.0e-6f;
  glm::vec2          vec;
  Thrust(float x, float y) 
    : vec(x, y) {
  }
};

struct Attitude {
  static constexpr float unit = -9.0e-6f;
  float          dir;
  Attitude(float d) 
    : dir(d) {
  }
};

struct Shield {
  float mag;
  Shield(float m) 
    : mag(m) {
  }
};

struct Fuel {
  float mag;
  Fuel(float m) 
    : mag(m) {
  }
};

struct GlVbo : boson::Component<GlVbo> {
  pronghorn::VertexBufferObject vbo;
  GlVbo(GLsizeiptr sizeBytes, const GLvoid* pData = 0, GLenum usage = pronghorn::VertexBufferObject::STATIC) 
  : vbo(sizeBytes, pData, usage) {
  }
};

struct GlTexture : boson::Component<GlTexture> {
  pronghorn::Texture texture;
  GlTexture(GLsizei       width, 
            GLsizei       height, 
            GLenum        fmt, 
            const GLvoid* pData)
  : texture(width, height, fmt, pData) {
  }
};

struct GlProgram : boson::Component<GlProgram> {
  pronghorn::Program program;
  GlProgram(pronghorn::VertexShader   vertexShader, 
            pronghorn::FragmentShader fragShader,
            const GLenum   primitiveType = pronghorn::Program::defaultPrimitiveType_)
  : program (vertexShader, fragShader, primitiveType) {
  }
     
};

struct Transform : boson::Component<Transform> {
  glm::mat4  transform;
  Transform* parent;
  Transform(Transform* p = 0) : parent(p) {}
  unsigned depth() const {
    if (!parent) { 
      return 0; 
    }

    return 1 + parent->depth();
  } 
  bool operator<(const Transform& rhs) const {
    return depth() > rhs.depth();
  }
};

template <typename T>
struct Track : boson::Component<Track<T>> {
  T* value;
  Track(T* v = 0) : value(v) {}
};

//------------------------------------------------------------------------------

} // namespace 

//------------------------------------------------------------------------------

#endif
