//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_TEXT_H
#define INCLUDED_TEXT_H

#ifndef INCLUDED_SSTREAM
#define INCLUDED_SSTREAM
#include <sstream>
#endif

#ifndef INCLUDED_STDEXCEPT
#define INCLUDED_STDEXCEPT
#include <stdexcept>
#endif

#ifndef INCLUDED_OPTIONAL_H
#include "TimeMs.h"
#endif

#ifndef INCLUDED_RENDERABLE_H
#include "Renderable.h"
#endif

#ifndef INCLUDED_PARAMS_H
#include "Params.h"
#endif

#include "Validate.h"

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

static void 
toStream(std::stringstream& stream, const char* msg) {
  while (*msg) {
    if (*msg == '%' && *(++msg) != '%') {
      throw std::runtime_error("invalid format string: missing arguments");
    }
    stream << *msg++;
  }
}

template<typename H, typename... T>
static void 
toStream(std::stringstream& stream, const char* msg, H value, T... args) {
  while (*msg) {
    if (*msg == '%' && *(++msg) != '%') {
      stream << value;
      toStream(stream, msg, args...); // call even when *s == 0 to detect extra arguments
      return;
    }
    stream << *msg++;
  }
  throw std::logic_error("extra arguments provided to toStream");
}

//------------------------------------------------------------------------------

struct Text : public Quad 
            , public TimeDelta
            , public TimeMs
            , public Position
            , public Validate
            , public Expiration {

  Text(float x, float y);
  ~Text();

  template <class...T>
  void setText(const char* pMsg, T... args) {
  std::stringstream stream;
    roidrage::toStream(stream, pMsg, args...);
    text_ = stream.str();
  }

  const std::string& 
  getText() const {
    return text_;
  }
  
  void fade();

protected:
  virtual void preRender(OrthoCamera* pCam);
  virtual void onRender(OrthoCamera* pCam);
  virtual void postRender(OrthoCamera* pCam);

private:
  std::string text_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
