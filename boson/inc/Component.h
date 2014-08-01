//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_COMPONENT_H
#define INCLUDED_COMPONENT_H

#include <string>

//------------------------------------------------------------------------------

namespace boson {

//------------------------------------------------------------------------------

struct System;
struct IComponent {
  virtual ~IComponent() {}
  virtual std::string toString() = 0;
};

template <typename T>
struct Component : public IComponent {
  virtual ~Component() {}
  // knows what type it is
  virtual std::string toString() {
    return "???";
  }

  bool operator<(const Component& other) const {
    return this < &other; 
  }
};
#define COMPONENT(n) struct n : public Component<n>

//------------------------------------------------------------------------------

}

//------------------------------------------------------------------------------

#endif
